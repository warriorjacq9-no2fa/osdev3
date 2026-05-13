#include <kernel/kshell.h>
#include <kernel/kevent.h>
#include <kernel/kmalloc.h>
#include <kernel/klog.h>
#include <string.h>
#include <stdio.h>

static size_t len, hist_len;

static size_t shellptr;
static char* shellbuf;
static char** shellhist;

void kshell_proc(char* s) {
    kprintf(LOG_INFO, "kernel", "You typed %s\r\n", s);
}

void kconsumer_shell(kevent_input_t *evt) {
    putc(evt->ch.character);
    shellbuf[shellptr++] = evt->ch.character;
    if(shellptr >= len || (evt->ch.character == '\n')) {
        shellbuf[shellptr] = '\0';
        kshell_proc(shellbuf);
        shellptr = 0;
        kfree(shellhist[hist_len - 1]);
        memmove(shellhist + 1, shellhist, sizeof(char*) * (hist_len - 1));
        shellhist[0] = shellbuf;
        shellbuf = kmalloc(len + 1, 0);
    }
}

int kshell_init(size_t _len, size_t _hist_len) {
    len = _len;
    hist_len = _hist_len;
    shellptr = 0;
    shellbuf = kmalloc(len + 1, 0);
    if(shellbuf == NULL) return 1;
    memset(shellbuf, 0, len);

    shellhist = kmalloc(sizeof(char*) * hist_len, 0);
    if(shellhist == NULL) {
        kfree(shellbuf);
        return 1;
    }
    memset(shellhist, 0, sizeof(char*) * hist_len);

    kevent_consumer_t consumer = {
        .callback = kconsumer_shell,
        .type = KEVENT_CHAR
    };
    if(kevent_register(consumer)) {
        kfree(shellbuf);
        kfree(shellhist);
        return 1;
    }
    return 0;
}