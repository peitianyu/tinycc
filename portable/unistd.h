/* portable/unistd.h — POSIX unistd extensions for Windows
 * Include with: -Iportable, Usage: #include <unistd.h>
 *
 * MSVCRT provides basic unistd via <io.h>. This adds missing POSIX:
 * getline, usleep, getopt, getpid, sleep, pread, pwrite, fsync.
 */

#ifndef _PORTABLE_UNISTD_H
#define _PORTABLE_UNISTD_H

#include <io.h>       /* MSVCRT's base unistd */
#include <stdio.h>    /* for FILE* in getline */
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Process */
int getpid(void);
unsigned int sleep(unsigned int seconds);
int usleep(unsigned int usec);

/* File descriptor I/O */
ssize_t pread(int fd, void *buf, size_t count, off_t offset);
ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);
int fsync(int fd);

/* Line reading */
ssize_t getline(char **lineptr, size_t *n, FILE *stream);

/* getopt */
extern char *optarg;
extern int optind, opterr, optopt;
int getopt(int argc, char *const argv[], const char *optstring);

/* Stub: fork/exec (not supported) */
#include <sys/types.h>
#if !defined(pid_t) && !defined(_PID_T_DEFINED) && !defined(_PID_T_)
typedef int pid_t;
#endif
pid_t fork(void);
/* execvp/execv are declared in <process.h> by MSVCRT/TCC.
   Our implementations are in port.c. */

/* Stub: epoll */
int epoll_create(int size);
int epoll_wait(int epfd, void *events, int maxevents, int timeout);

/* File operations */
int ftruncate(int fd, off_t length);

/* symlink / readlink / link */
int symlink(const char *target, const char *linkpath);
ssize_t readlink(const char *pathname, char *buf, size_t bufsiz);
int link(const char *oldpath, const char *newpath);

/* chown (stub: no Unix ownership on Windows) */
int chown(const char *pathname, unsigned int owner, unsigned int group);

#ifdef __cplusplus
}
#endif

#endif
