/* portable/stdio.h — POSIX stdio additions for Windows
 * Chains via #include_next to the system <stdio.h> and adds
 * POSIX functions missing from MSVCRT.
 */

#ifndef _PORTABLE_STDIO_H
#define _PORTABLE_STDIO_H

#include_next <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

int asprintf(char **strp, const char *fmt, ...);
int vasprintf(char **strp, const char *fmt, va_list ap);

#ifdef __cplusplus
}
#endif

#endif
