#include <string.h>

char *strcat(char* s, const char* append) {
	char* save = s;

	for(; *s; ++s);
	while((*s++ = *append++) != '\0');
	return save;
}