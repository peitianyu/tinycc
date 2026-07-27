/* portable/sys/select.h — POSIX select() with console handle support
 * Include with: -Iportable
 * Usage: #include <sys/select.h>  (standard, unchanged)
 */

#ifndef _PORTABLE_SYS_SELECT_H
#define _PORTABLE_SYS_SELECT_H

#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FD_SETSIZE
#define FD_SETSIZE 64
#endif

/* On Windows with <windows.h> already included, fd_set comes from winsock.
 * Only define our own when winsock hasn't been included yet. */
#if !defined(_WINSOCKAPI_) && !defined(_WINSOCK2API_)

typedef struct {
    unsigned int fd_count;
    unsigned int fd_array[FD_SETSIZE];
} fd_set;

void FD_ZERO_(fd_set *s);
void FD_SET_(int fd, fd_set *s);
void FD_CLR_(int fd, fd_set *s);
int  FD_ISSET_(int fd, fd_set *s);

#define FD_ZERO(s)   FD_ZERO_(s)
#define FD_SET(f,s)  FD_SET_((f),(s))
#define FD_CLR(f,s)  FD_CLR_((f),(s))
#define FD_ISSET(f,s) FD_ISSET_((f),(s))

#endif

/* Our select() wraps platform select, adding console handle support.
 * On Windows: renamed to avoid collision with winsock's select. */
int __port_select(int nfds, fd_set *readfds, fd_set *writefds,
                  fd_set *exceptfds, struct timeval *timeout);
#define select __port_select

#ifdef __cplusplus
}
#endif

#endif
