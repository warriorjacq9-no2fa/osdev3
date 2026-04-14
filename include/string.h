#ifndef STRING_H
#define STRING_H

#include <stddef.h>

void* memcpy(void* restrict dstptr, const void* restrict srcptr, size_t size);
void* memmove(void* dstptr, const void* srcptr, size_t size);
void* memset(void* bufptr, int val, size_t size);

char* strcat(char* s, const char* append);
char* strchr(const char* p, int ch);
char* strcpy(char* d, const char* s);
char* strncpy(char* d, const char* s, size_t n);
size_t strlen(const char* s);
size_t strnlen(const char* s, size_t n);

#endif