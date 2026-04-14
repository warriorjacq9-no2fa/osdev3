#include <string.h>

size_t strlen(const char* s) {
    size_t n = 0;
    while(s[n]) n++;
    return n;
}

size_t strnlen(const char* s, size_t n) {
    size_t sn = 0;
    while(s[sn] && sn != n) n++;
    return n;
}