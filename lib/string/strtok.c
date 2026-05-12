#include <stddef.h>
#include <string.h>

char *strtok_r(char *s, const char *delim, char **saveptr)
{
    char *token_start;

    if (s == NULL)
        s = *saveptr;

    if (s == NULL || *s == '\0') {
        *saveptr = s;
        return NULL;
    }

    s += strspn(s, delim);
    if (*s == '\0') {
        *saveptr = s;
        return NULL;
    }

    token_start = s;

    s += strcspn(s, delim);

    if (*s != '\0')
        *s++ = '\0';

    *saveptr = s;
    return token_start;
}