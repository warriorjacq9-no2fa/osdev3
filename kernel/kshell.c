#include <kernel/kshell.h>
#include <kernel/kevent.h>
#include <kernel/kmalloc.h>
#include <kernel/klog.h>
#include <string.h>
#include <stdio.h>
#include <ansi.h>
#include <fs/vfs.h>
#include <fs/ext2.h>

#define PROMPT(n) (n "$ ")

static ssize_t len, hist_len;
static size_t hist_count;

static size_t shellptr;
static ssize_t histptr;
static char* shellbuf;
static char* savebuf;
static char** shellhist;

enum ansi_state {
    S_NONE,
    S_ESC,
    S_CSI
};

enum kshell_commands {
    KC_NONE,
    KC_READ,
    KC_STAT
};

static int state;

const char *mode_to_string(int mode) {
    static char str[11];

    // File type
    str[0] =
        S_ISREG(mode)  ? '-' :
        S_ISDIR(mode)  ? 'd' :
        S_ISLNK(mode)  ? 'l' :
        S_ISCHR(mode)  ? 'c' :
        S_ISBLK(mode)  ? 'b' :
        S_ISFIFO(mode) ? 'p' :
        S_ISSOCK(mode) ? 's' :
                          '?';

    // Owner permissions
    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    str[3] = (mode & S_IXUSR) ? 'x' : '-';

    // Group permissions
    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    str[6] = (mode & S_IXGRP) ? 'x' : '-';

    // Other permissions
    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    str[9] = (mode & S_IXOTH) ? 'x' : '-';

    str[10] = '\0';

    return str;
}

int kshell_hash(char* cmd) {
    if(strcmp(cmd, "read") == 0) return KC_READ;
    if(strcmp(cmd, "stat") == 0) return KC_STAT;
    return KC_NONE;
}

void kshell_proc(char* str) {
    char* s = strdup(str);
    char* save;
    char* cmd = strtok_r(s, " ", &save);
    if(cmd == NULL) return;
    switch(kshell_hash(cmd)) {
        case KC_READ:
            char* fp = strtok_r(NULL, " ", &save);
            if(fp == NULL) {
                kprintf(LOG_WARN, "kernel", "Usage: read <filepath>\r\n");
                break;
            }
            vnode_t fd;
            int res;
            if((res = ext2_open(&fd, fp, O_RDONLY)) < 0) {
                if(res == -ENOENT)
                    kprintf(LOG_WARN, "kernel", "File not found: %s\r\n", fp);
                else
                    kprintf(LOG_WARN, "kernel", "Stat failed with code %d\r\n", res);
                break;
            }
            stat_t data;
            if((res = fd.ops->fstat(&fd, &data)) < 0) {
                kprintf(LOG_WARN, "kernel", "Read (Stat) failed with code %d\r\n", res);
                break;
            }
            char* fdata = kmalloc(data.st_size + 1, 0);
            if((res = fd.ops->read(&fd, fdata, 0, data.st_size)) < 0) {
                kprintf(LOG_WARN, "kernel", "Read failed with code %d\r\n", res);
                break;
            }
            fdata[data.st_size] = '\0';
            printf("%s\r\n", fdata);
            break;
        case KC_STAT:
            fp = strtok_r(NULL, " ", &save);
            if(fp == NULL) {
                kprintf(LOG_WARN, "kernel", "Usage: stat <filepath>\r\n");
                break;
            }
            if((res = ext2_stat(fp, &data)) < 0) {
                if(res == -ENOENT)
                    kprintf(LOG_WARN, "kernel", "File not found: %s\r\n", fp);
                else
                    kprintf(LOG_WARN, "kernel", "Stat failed with code %d\r\n", res);
                break;
            }
            kprintf(LOG_INFO, "kernel", "%s %u %04u:%04u % 12u %s\r\n",
                mode_to_string(data.st_mode), data.st_nlink,
                data.st_uid, data.st_gid, data.st_size, fp
            );
            break;
        case KC_NONE:
            kprintf(LOG_WARN, "kernel", "Not a command: %s\r\n", cmd);
            break;
    }
    kfree(s);
}

void hist_newline() {
    // Don't store empty or duplicate commands
    if(shellptr == 0) return;
    if(hist_count > 0 && strcmp(shellhist[0], shellbuf) == 0) {
        shellptr = 0;
        shellbuf[0] = '\0';
        histptr = -1;
        return;
    }

    // Evict the oldest entry if the history is full
    size_t slots = (size_t)hist_len;
    if(hist_count == slots) {
        kfree(shellhist[slots - 1]);
        hist_count--;
    }

    // Shift everything down to make room at index 0 (most recent)
    memmove(&shellhist[1], &shellhist[0], sizeof(char*) * hist_count);

    shellhist[0] = strdup(shellbuf);

    if(shellhist[0] != NULL)
        hist_count++;

    // Reset input state
    shellptr = 0;
    shellbuf[0] = '\0';
    histptr = -1;
    savebuf[0] = '\0';
}

void hist_move(int direction) {
    // direction > 0 = up (older), direction < 0 = down (newer)
    if(direction > 0) {
        // Going up: save current input on first move, then walk back
        if(histptr == -1)
            strncpy(savebuf, shellbuf, len);

        ssize_t next = histptr + 1;
        if(next >= (ssize_t)hist_count) return; // already at oldest

        histptr = next;
        strncpy(shellbuf, shellhist[histptr], len);
        shellbuf[len] = '\0';
    } else {
        // Going down: walk forward toward present
        if(histptr == -1) return; // already at live input, nothing to do

        ssize_t next = histptr - 1;
        if(next < 0) {
            // Back to the live input the user was typing
            histptr = -1;
            strncpy(shellbuf, savebuf, len);
            shellbuf[len] = '\0';
        } else {
            histptr = next;
            strncpy(shellbuf, shellhist[histptr], len);
            shellbuf[len] = '\0';
        }
    }

    shellptr = strlen(shellbuf);
}

void kconsumer_shell(kevent_input_t *evt) {
    char c = evt->ch.character;
    if(c == '\r') c = '\n'; // Enter on terminal emulators sends a carriage return
    switch(state) {
        case S_NONE:
            if(c == '\e') {
                state = S_ESC;
                break;
            }
            if(c == '\n') {
                puts("\n");

                kshell_proc(shellbuf);
                hist_newline();

                puts(PROMPT(""));
                break;
            }
            if(c == '\b' || c == '\x7f') {
                if(shellptr <= 0) {
                    putc('\a');
                    break;
                }
                shellbuf[--shellptr] = '\0';
                puts(PROMPT("\r\033[2K"));
                puts(shellbuf);
                break;
            }

            if(shellptr < len - 1) {
                shellbuf[shellptr++] = c;
                shellbuf[shellptr] = '\0';
            }
            puts(PROMPT("\r\033[2K"));
            puts(shellbuf);
            break;
        case S_ESC:
            if(c == '[') {
                state = S_CSI;
            } else {
                state = S_NONE;
            }
            break;
        case S_CSI:
            state = S_NONE;
            switch(c) {
                case 'A':
                    hist_move(1);
                    break;
                case 'B':
                    hist_move(-1);
                    break;
            }
            puts(PROMPT("\r\033[2K"));
            puts(shellbuf);
            break;
    }
}

void kconsumer_arrows(kevent_input_t *evt) {
    if(evt->key.pressed) {
        switch(evt->key.keycode) {
            case KEY_UP:
                hist_move(1);
                break;
            case KEY_DOWN:
                hist_move(-1);
                break;
        }
    }
    puts(PROMPT("\r\033[2K"));
    puts(shellbuf);
}

int kshell_init(size_t _len, size_t _hist_len) {
    state = S_NONE;
    len = _len;
    hist_len = _hist_len;
    shellptr = 0;
    hist_count = 0;
    shellbuf = kmalloc(len + 1, 0);
    if(shellbuf == NULL) return 1;
    savebuf = kmalloc(len + 1, 0);
    if(savebuf == NULL) return 1;
    memset(shellbuf, 0, len + 1);
    memset(savebuf, 0, len + 1);

    histptr = -1;
    shellhist = kmalloc(sizeof(char*) * hist_len, 0);
    if(shellhist == NULL) {
        kfree(shellbuf);
        return 1;
    }
    memset(shellhist, 0, sizeof(char*) * hist_len);

    kevent_consumer_t consumer_shell = {
        .callback = kconsumer_shell,
        .type = KEVENT_CHAR
    };
    kevent_consumer_t consumer_arrows = {
        .callback = kconsumer_arrows,
        .type = KEVENT_KEY
    };

    if(kevent_register(consumer_shell) || kevent_register(consumer_arrows)) {
        kfree(shellbuf);
        kfree(shellhist);
        return 1;
    }
    puts(PROMPT("\r\033[2K"));
    return 0;
}