#include <stdio.h>

void kinput(char c) {
    if(c >= ' ' && c <= '~') putc(c);
    else if(c == '\e') putc('^');
}