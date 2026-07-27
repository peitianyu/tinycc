/* portable/sys/time.h — POSIX struct timeval
 * Include with: -Iportable
 * Usage: #include <sys/time.h>  (standard, unchanged)
 */

#ifndef _PORTABLE_SYS_TIME_H
#define _PORTABLE_SYS_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _TIMEVAL_DEFINED
struct timeval {
    long tv_sec;
    long tv_usec;
};
#endif

#ifndef _TIMEZONE_DEFINED
struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};
#endif

#ifndef _GETTIMEOFDAY_DEFINED
int gettimeofday(struct timeval *__restrict__, void *__restrict__);
#endif

#ifdef __cplusplus
}
#endif

#endif
