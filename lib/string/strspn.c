#include <stddef.h>
#include <string.h>

size_t strspn(char* s, const char* accept) {
    char* p;
    const char* a;
    size_t count = 0;

    for(p = s; *p != '\0'; ++p) {
        for(a = accept; *a != '\0'; ++a)
            if(*p == *a) break;
        if(*a == '\0') return count;
        ++count;
    }
    return count;
}

size_t strcspn(const char* s, const char* reject) {
    unsigned int len = 0;
    if((s == NULL) || (reject == NULL))
        return len;
    while(*s)
    {
        if(strchr(s,*reject))
        {
            return len;
        }
        else
        {
            s++;
            len++;
        }
    }
    return len;
}