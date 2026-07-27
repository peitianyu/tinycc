/* portable/stdlib.h — POSIX stdlib additions for Windows
 * Chains via #include_next to the system <stdlib.h> and adds
 * POSIX functions missing from MSVCRT.
 */

#ifndef _PORTABLE_STDLIB_H
#define _PORTABLE_STDLIB_H

/* TCC defines __TINYC__ as "9.2", but MinGW's <stdlib.h> uses
   #if __TINYC__ which needs an integer. Temporarily override it. */
#pragma push_macro("__TINYC__")
#undef __TINYC__
#define __TINYC__ 1
#include_next <stdlib.h>
#pragma pop_macro("__TINYC__")
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

int    setenv(const char *name, const char *value, int overwrite);
int    unsetenv(const char *name);
char  *realpath(const char *path, char *resolved);
int    mkstemp(char *templ);
int    posix_memalign(void **memptr, size_t alignment, size_t size);
void   posix_memalign_free(void *p);
long   random(void);
void   srandom(unsigned int seed);

#ifdef __cplusplus
}
#endif

#endif
