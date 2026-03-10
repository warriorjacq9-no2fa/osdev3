#include <kernel/klog.h>
#include <string.h>
#include <stdint.h>

void klog(char level, const char *namespace, const char* s) {
    char prefix[32];

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

    const char* line = strcat(prefix, s);
    puts(line);
}