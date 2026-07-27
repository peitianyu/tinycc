/* portable/dirent.h — POSIX dirent extensions for Windows
 * Include with: -Iportable
 * Usage: #include <dirent.h>  (standard, unchanged)
 *
 * MSVCRT/tinycc have basic dirent. This adds scandir/alphasort.
 */

#ifndef _PORTABLE_DIRENT_H
#define _PORTABLE_DIRENT_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Minimal dirent if not already available via system */
#ifndef _DIRENT_H_
struct dirent {
    char d_name[260];
};
typedef struct _DIR DIR;
DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);
#endif

/* Extended functions */
int alphasort(const struct dirent **a, const struct dirent **b);
int scandir(const char *dirp, struct dirent ***namelist,
            int (*filter)(const struct dirent *),
            int (*compar)(const struct dirent **, const struct dirent **));

#ifdef __cplusplus
}
#endif

#endif
