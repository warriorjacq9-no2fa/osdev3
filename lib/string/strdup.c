#include <stddef.h>
#include <string.h>
#include <kernel/kmalloc.h>

char* strdup(char* s) {
    size_t len = strlen(s) + 1;
    char* d = kmalloc(len, 0);
    if(d == NULL) return NULL;
    return (char*)memcpy(d, s, len);
}