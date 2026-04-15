#include <string.h>

char* strcpy(char* d, const char* s) {
    char* dest = d;
    const char* src = s;
    while((*dest++ = *src++));
    return dest;
}

char* strncpy(char* d, const char* s, size_t n) {
    size_t i = 0;

    for (; i < n && s[i]; i++)
        d[i] = s[i];

    for (; i < n; i++)
        d[i] = '\0';

    return d;
}