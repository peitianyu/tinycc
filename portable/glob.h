/* portable/glob.h — POSIX glob() for Windows
 * Include with: -Iportable, Usage: #include <glob.h>
 * Uses FindFirstFile/FindNextFile.
 */

#ifndef _PORTABLE_GLOB_H
#define _PORTABLE_GLOB_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GLOB_ERR      0x01
#define GLOB_NOMATCH  0x02
#define GLOB_NOSORT   0x04

typedef struct {
    size_t  gl_pathc;
    char  **gl_pathv;
    size_t  gl_offs;
} glob_t;

int glob(const char *pattern, int flags,
         int (*errfunc)(const char *epath, int eerrno),
         glob_t *pglob);
void globfree(glob_t *pglob);

#ifdef __cplusplus
}
#endif

#endif
