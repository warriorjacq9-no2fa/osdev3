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

static size_t len, hist_len;

static size_t shellptr;
static size_t histptr;
static char* shellbuf;
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
    switch(kshell_hash(cmd)) {
        case KC_READ:
            char* arg1 = strtok_r(NULL, " ", &save);
            if(arg1 == NULL) {
                kprintf(LOG_WARN, "kernel", "Usage: read <filepath>\r\n");
                break;
            }
            kprintf(LOG_INFO, "kernel", "Read command %s\r\n", arg1);
            break;
        case KC_STAT:
            char* fp = strtok_r(NULL, " ", &save);
            if(fp == NULL) {
                kprintf(LOG_WARN, "kernel", "Usage: stat <filepath>\r\n");
                break;
            }
            stat_t data;
            int res;
            if((res = ext2_stat(fp, &data)) < 0) {
                if(res == -ENOENT)
                    kprintf(LOG_WARN, "kernel", "File not found: %s\r\n", fp);
                else
                    kprintf(LOG_WARN, "kernel", "stat failed with code %d\r\n", res);
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

void kconsumer_shell(kevent_input_t *evt) {
    char c = evt->ch.character;
    if(c == '\r') c = '\n'; // Enter on terminal emulators sends a carriage return
    switch(state) {
        case S_NONE:
            if(c == '\e') {
                state = S_ESC;
                break;
            }
            shellbuf[shellptr++] = c;
            if(shellptr >= len) shellptr = len - 1;
            puts(PROMPT("\r\033[2K"));
            puts(shellbuf);
            if(c == '\n') {
                shellbuf[shellptr - 1] = '\0';
                kshell_proc(shellbuf);
                shellptr = 0;
                kfree(shellhist[hist_len - 1]);
                memmove(shellhist + 1, shellhist, sizeof(char*) * (hist_len - 1));
                shellbuf = kmalloc(len + 1, 0);
                memset(shellbuf, 0, len + 1);
                shellhist[0] = shellbuf;
                puts(PROMPT("\r\033[2K"));
            }
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
                    if(histptr >= hist_len - 1 || !(shellhist[histptr + 1])) break;
                    shellbuf = shellhist[++histptr];
                    shellptr = strlen(shellbuf);
                    break;
                case 'B':
                    if(histptr <= 0) break;
                    shellbuf = shellhist[--histptr];
                    shellptr = strlen(shellbuf);
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
                if(histptr >= hist_len - 1 || !(shellhist[histptr + 1])) break;
                shellbuf = shellhist[++histptr];
                shellptr = strlen(shellbuf);
                break;
            case KEY_DOWN:
                if(histptr <= 0) break;
                shellbuf = shellhist[--histptr];
                shellptr = strlen(shellbuf);
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
    shellbuf = kmalloc(len + 1, 0);
    if(shellbuf == NULL) return 1;
    memset(shellbuf, 0, len + 1);

    histptr = 0;
    shellhist = kmalloc(sizeof(char*) * hist_len, 0);
    if(shellhist == NULL) {
        kfree(shellbuf);
        return 1;
    }
    memset(shellhist, 0, sizeof(char*) * hist_len);
    shellhist[0] = shellbuf;

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