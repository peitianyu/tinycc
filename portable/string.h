/* portable/string.h — POSIX string additions for Windows
 * Chains via #include_next to the system <string.h> and adds
 * POSIX functions missing from MSVCRT.
 */

#ifndef _PORTABLE_STRING_H
#define _PORTABLE_STRING_H

#include_next <string.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

char *strdup(const char *s);
char *strndup(const char *s, size_t n);
char *strsep(char **stringp, const char *delim);
void *memrchr(const void *s, int c, size_t n);
char *strcasestr(const char *haystack, const char *needle);
int   strerror_r(int errnum, char *buf, size_t buflen);

#ifdef __cplusplus
}
#endif

#endif
