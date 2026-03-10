#ifndef STRING_H
#define STRING_H

#include <stddef.h>

#ifndef __has_include
#warning "This compiler does not support __has_include, some features will be missing"
#define __has_include(x) 0
#endif

void* memmove(void* dstptr, const void* srcptr, size_t size);
void* memset(void* bufptr, int val, size_t size);

char *strcat(char *s, const char *append);
char* strcpy(char* d, const char* s);
char* strncpy(char* d, const char* s, size_t n);
size_t strlen(const char *s);

#endif