#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>

void puts(const char* s);
void putc(char c);

int printf(const char* format, ...);
int vprintf(const char* format, va_list list);
int sprintf(char* str, const char* format, ...);
int vsprintf(char* str, const char* format, va_list list);

#endif