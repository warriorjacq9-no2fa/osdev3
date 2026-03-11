#include <string.h>

char* __strchr_chk(const char *p, int ch, size_t s_len) {
	for (;; ++p, s_len--) {
		if (s_len == 0)
            return (char*)NULL;
		if (*p == (char) ch)
			return (char *)p;
		if (!*p)
			return (char *)NULL;
	}
}
char* strchr(const char *p, int ch) {
    return __strchr_chk(p, ch, (size_t) -1);
}