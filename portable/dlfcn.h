/* portable/dlfcn.h — POSIX dlopen/dlsym/dlclose/dlerror for Windows
 * Include with: -Iportable
 * Usage: #include <dlfcn.h>  (standard, unchanged)
 *
 * Maps to LoadLibrary/GetProcAddress/FreeLibrary/GetLastError.
 */

#ifndef _PORTABLE_DLFCN_H
#define _PORTABLE_DLFCN_H

#ifdef __cplusplus
extern "C" {
#endif

#define RTLD_LAZY   0x0001
#define RTLD_NOW    0x0002
#define RTLD_LOCAL  0x0000
#define RTLD_GLOBAL 0x0100

void *dlopen(const char *filename, int flags);
void *dlsym(void *handle, const char *symbol);
int   dlclose(void *handle);
char *dlerror(void);

#ifdef __cplusplus
}
#endif

#endif
