/* portable/poll.h — POSIX poll() for Windows
 * Include with: -Iportable, Usage: #include <poll.h>
 * Wraps WSAPoll / WaitForMultipleObjects for console.
 */

#ifndef _PORTABLE_POLL_H
#define _PORTABLE_POLL_H

#ifdef __cplusplus
extern "C" {
#endif

#define POLLIN  0x001
#define POLLOUT 0x004
#define POLLERR 0x008
#define POLLHUP 0x010
#define POLLNVAL 0x020

typedef unsigned long nfds_t;

struct pollfd {
    int   fd;
    short events;
    short revents;
};

int poll(struct pollfd *fds, nfds_t nfds, int timeout_ms);

#ifdef __cplusplus
}
#endif

#endif
