#include <kernel/klog.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

char prefix[32];
char* klog_prefix(char level, const char* namespace) {
    char *p = prefix;

    *p++ = '[';

    while(*namespace) *p++ = *namespace++;

    *p++ = ']';
    *p++ = ' ';

    switch(level) {
        case LOG_INFO:
            *p++ = 'I';
            *p++ = 'N';
            *p++ = 'F';
            *p++ = 'O';
            break;
        
        case LOG_WARN:
            *p++ = 'W';
            *p++ = 'A';
            *p++ = 'R';
            *p++ = 'N';
            break;

        case LOG_ERR:
            *p++ = ' ';
            *p++ = 'E';
            *p++ = 'R';
            *p++ = 'R';
            break;
    }

    *p++ = ':';
    *p++ = ' ';
    *p = '\0';

    return prefix;
}

void klog(char level, const char* namespace, const char* s) {
    puts(klog_prefix(level, namespace));
    puts(s);
}

void kprintf(char level, const char* namespace, const char* fmt, ...) {
    puts(klog_prefix(level, namespace));
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}