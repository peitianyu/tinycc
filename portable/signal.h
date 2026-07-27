/* portable/signal.h — POSIX-compatible signal.h for Windows
 * Include with: -Iportable
 * Usage: #include <signal.h>  (standard, unchanged — our header shadows system)
 *
 * Provides the same symbols as MSVCRT's <signal.h> plus POSIX extensions:
 * SIGWINCH, struct sigaction, sigaction().
 *
 * When compiled with -Iportable, this file is found BEFORE the toolchain's
 * <signal.h>. It provides all necessary symbols so the real one isn't needed.
 */

#ifndef _PORTABLE_SIGNAL_H
#define _PORTABLE_SIGNAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* ─── Base types and constants (MSVCRT-compatible) ─── */

typedef int sig_atomic_t;

#define NSIG 23

#define SIGHUP    1
#define SIGINT    2
#define SIGQUIT   3
#define SIGILL    4
#define SIGTRAP   5
#define SIGIOT    6
#define SIGABRT   6
#define SIGEMT    7
#define SIGFPE    8
#define SIGKILL   9
#define SIGBUS    10
#define SIGSEGV   11
#define SIGSYS    12
#define SIGPIPE   13
#define SIGALRM   14
#define SIGTERM   15
#define SIGBREAK  21
#define SIGABRT2  22

typedef void (*__p_sig_fn_t)(int);

#define SIG_DFL ((__p_sig_fn_t)0)
#define SIG_IGN ((__p_sig_fn_t)1)
#define SIG_ERR ((__p_sig_fn_t)-1)
#define SIG_GET ((__p_sig_fn_t)2)
#define SIG_SGE ((__p_sig_fn_t)3)
#define SIG_ACK ((__p_sig_fn_t)4)

__p_sig_fn_t signal(int _SigNum, __p_sig_fn_t _Func);
int raise(int _SigNum);

/* ─── POSIX extensions (normally missing from MSVCRT) ─── */

#define SIGWINCH 28

struct sigaction {
    void (*sa_handler)(int);
    int   sa_flags;
};
#define SA_RESTART 0x10000000

int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);

#ifdef __cplusplus
}
#endif

#endif
