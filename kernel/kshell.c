#include <kernel/kshell.h>
#include <kernel/kevent.h>
#include <kernel/kmalloc.h>
#include <kernel/klog.h>
#include <string.h>
#include <stdio.h>
#include <ansi.h>

#define PROMPT(n) (n "> ")

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
static int state;

void kshell_proc(char* s) {
    kprintf(LOG_INFO, "kernel", "You typed %s\r\n", s);
}

void kconsumer_shell(kevent_input_t *evt) {
    char c = evt->ch.character;
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
                    if(histptr >= len - 1 || !(shellhist[histptr + 1])) break;
                    shellbuf = shellhist[++histptr];
                    break;
                case 'B':
                    if(histptr <= 0) break;
                    shellbuf = shellhist[--histptr];
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
                if(histptr >= len - 1) break;
                char* temp = shellhist[++histptr];
                if(temp) shellbuf = temp;
                break;
            case KEY_DOWN:
                if(histptr <= 0) break;
                shellbuf = shellhist[--histptr];
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