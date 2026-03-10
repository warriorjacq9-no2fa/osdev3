#ifndef STRING_H
#define STRING_H

#include <stddef.h>

#ifndef __has_include
#warning "This compiler does not support __has_include, some features will be missing"
#define __has_include(x) 0
#endif

size_t strlen(const char *s);
char *strcat(char *s, const char *append);

#endif