/* portable/time.h — POSIX time extensions for Windows
 * Chains via #include_next to the system <time.h> and adds
 * POSIX functions missing from MSVCRT.
 */

#ifndef _PORTABLE_TIME_H
#define _PORTABLE_TIME_H

#include_next <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CLOCK_REALTIME  0
#define CLOCK_MONOTONIC 1

#if !defined(_STRUCT_TIMESPEC) && !defined(_TIMESPEC_DEFINED)
struct timespec {
    long tv_sec;
    long tv_nsec;
};
#endif

int nanosleep(const struct timespec *req, struct timespec *rem);
int clock_gettime(int clk_id, struct timespec *tp);

#ifdef __cplusplus
}
#endif

#endif
