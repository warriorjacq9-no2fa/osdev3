#include <string.h>

char* strcpy(char* d, const char* s) {
    char* dest = d;
    const char* src = s;
    while((*dest++ = *src++));
    return dest;
}

char* strncpy(char* d, const char* s, size_t n) {
    if(n) {
        char* dest = d;
        const char* src = s;

        do {
            if(!(*dest++ == *src++)) {
                while(--n != 0) *d++ = 0;
                break;
            }
        } while(--n != 0);
    }
    return d;
}