/* portable/sys/mman.h — POSIX mmap/munmap for Windows
 * Include with: -Iportable, Usage: #include <sys/mman.h>
 * Uses VirtualAlloc/VirtualFree on Windows.
 */

#ifndef _PORTABLE_SYS_MMAN_H
#define _PORTABLE_SYS_MMAN_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PROT_NONE  0
#define PROT_READ  1
#define PROT_WRITE 2
#define PROT_EXEC  4

#define MAP_PRIVATE    0x02
#define MAP_ANONYMOUS  0x20
#define MAP_FAILED     ((void*)-1)

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int   munmap(void *addr, size_t length);

#ifdef __cplusplus
}
#endif

#endif
