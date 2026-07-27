/* portable/sys/ioctl.h — POSIX ioctl() with TIOCGWINSZ
 * Include with: -Iportable
 * Usage: #include <sys/ioctl.h>  (standard, unchanged)
 */

#ifndef _PORTABLE_SYS_IOCTL_H
#define _PORTABLE_SYS_IOCTL_H

#ifdef __cplusplus
extern "C" {
#endif

struct winsize {
    unsigned short ws_row;
    unsigned short ws_col;
    unsigned short ws_xpixel;
    unsigned short ws_ypixel;
};

#define TIOCGWINSZ  0x5413
#define TIOCSWINSZ  0x5414

int ioctl(int fd, unsigned long request, ...);

#ifdef __cplusplus
}
#endif

#endif
