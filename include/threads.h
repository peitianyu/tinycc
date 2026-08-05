/* TCC <threads.h> — C11 threads API, backed by POSIX threads.
   On POSIX/Android the system <pthread.h> is used; the portable/
   polyfill provides pthread.h on Windows. */

#ifndef _TCC_THREADS_H
#define _TCC_THREADS_H

#include <pthread.h>
#include <time.h>

typedef pthread_t thrd_t;
typedef pthread_mutex_t mtx_t;
typedef pthread_cond_t cnd_t;
typedef pthread_key_t tss_t;
typedef pthread_once_t once_flag;

#define ONCE_FLAG_INIT PTHREAD_ONCE_INIT
#define TSS_DTOR_ITERATIONS 4
#define thread_local _Thread_local

enum {
    thrd_success = 0,
    thrd_busy = 1,
    thrd_error = 2,
    thrd_nomem = 3,
    thrd_timedout = 4
};

enum {
    mtx_plain = 0,
    mtx_recursive = 1,
    mtx_timed = 2
};

typedef int (*thrd_start_t)(void *);

/* threads */
int thrd_create(thrd_t *thr, thrd_start_t func, void *arg);
int thrd_join(thrd_t thr, int *res);
int thrd_detach(thrd_t thr);
thrd_t thrd_current(void);
int thrd_equal(thrd_t a, thrd_t b);
_Noreturn void thrd_exit(int res);
int thrd_sleep(const struct timespec *duration, struct timespec *remaining);
void thrd_yield(void);

/* mutexes */
int mtx_init(mtx_t *mtx, int type);
int mtx_lock(mtx_t *mtx);
int mtx_timedlock(mtx_t *mtx, const struct timespec *at);
int mtx_trylock(mtx_t *mtx);
int mtx_unlock(mtx_t *mtx);
void mtx_destroy(mtx_t *mtx);

/* condition variables */
int cnd_init(cnd_t *cond);
int cnd_signal(cnd_t *cond);
int cnd_broadcast(cnd_t *cond);
int cnd_wait(cnd_t *cond, mtx_t *mtx);
int cnd_timedwait(cnd_t *cond, mtx_t *mtx, const struct timespec *at);
void cnd_destroy(cnd_t *cond);

/* thread-specific storage */
void *tss_get(tss_t key);
int tss_set(tss_t key, void *val);
int tss_create(tss_t *key, void (*dtor)(void *));
void tss_delete(tss_t key);

/* call once */
void call_once(once_flag *flag, void (*func)(void));

#endif /* _TCC_THREADS_H */
