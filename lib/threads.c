/* TCC C11 <threads.h> implementation, backed by POSIX threads.
   Compiled into libtcc1.a as threads.o. */

#include <threads.h>
#include <pthread.h>
#include <time.h>
#include <sched.h>
#include <errno.h>
#include <stdlib.h>

typedef struct thrd_ctx {
    thrd_start_t fn;
    void *arg;
} thrd_ctx_t;

static void *thrd_runner(void *p)
{
    thrd_ctx_t *c = (thrd_ctx_t *)p;
    void *r = (void *)(intptr_t)c->fn(c->arg);
    free(c);
    return r;
}

int thrd_create(thrd_t *thr, thrd_start_t func, void *arg)
{
    thrd_ctx_t *c;
    if (!thr || !func)
        return thrd_error;
    c = (thrd_ctx_t *)malloc(sizeof *c);
    if (!c)
        return thrd_nomem;
    c->fn = func;
    c->arg = arg;
    if (pthread_create(thr, NULL, thrd_runner, c) != 0) {
        free(c);
        return thrd_error;
    }
    return thrd_success;
}

int thrd_join(thrd_t thr, int *res)
{
    void *r;
    if (pthread_join(thr, &r) != 0)
        return thrd_error;
    if (res)
        *res = (int)(intptr_t)r;
    return thrd_success;
}

int thrd_detach(thrd_t thr)
{
    return pthread_detach(thr) == 0 ? thrd_success : thrd_error;
}

thrd_t thrd_current(void)
{
    return pthread_self();
}

int thrd_equal(thrd_t a, thrd_t b)
{
    return pthread_equal(a, b);
}

_Noreturn void thrd_exit(int res)
{
    pthread_exit((void *)(intptr_t)res);
}

int thrd_sleep(const struct timespec *duration, struct timespec *remaining)
{
    int e = nanosleep(duration, remaining);
    return e == 0 ? 0 : errno;
}

void thrd_yield(void)
{
    sched_yield();
}

int mtx_init(mtx_t *mtx, int type)
{
    pthread_mutexattr_t attr;
    int e;
    pthread_mutexattr_init(&attr);
    if (type & mtx_recursive)
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    e = pthread_mutex_init(mtx, &attr);
    pthread_mutexattr_destroy(&attr);
    return e == 0 ? thrd_success : thrd_error;
}

int mtx_lock(mtx_t *mtx)
{
    return pthread_mutex_lock(mtx) == 0 ? thrd_success : thrd_error;
}

int mtx_timedlock(mtx_t *mtx, const struct timespec *at)
{
    int e = pthread_mutex_timedlock(mtx, at);
    if (e == 0)
        return thrd_success;
    return e == ETIMEDOUT ? thrd_timedout : thrd_error;
}

int mtx_trylock(mtx_t *mtx)
{
    int e = pthread_mutex_trylock(mtx);
    if (e == 0)
        return thrd_success;
    return e == EBUSY ? thrd_busy : thrd_error;
}

int mtx_unlock(mtx_t *mtx)
{
    return pthread_mutex_unlock(mtx) == 0 ? thrd_success : thrd_error;
}

void mtx_destroy(mtx_t *mtx)
{
    pthread_mutex_destroy(mtx);
}

int cnd_init(cnd_t *cond)
{
    return pthread_cond_init(cond, NULL) == 0 ? thrd_success : thrd_error;
}

int cnd_signal(cnd_t *cond)
{
    return pthread_cond_signal(cond) == 0 ? thrd_success : thrd_error;
}

int cnd_broadcast(cnd_t *cond)
{
    return pthread_cond_broadcast(cond) == 0 ? thrd_success : thrd_error;
}

int cnd_wait(cnd_t *cond, mtx_t *mtx)
{
    int e = pthread_cond_wait(cond, mtx);
    return e == 0 ? thrd_success : thrd_error;
}

int cnd_timedwait(cnd_t *cond, mtx_t *mtx, const struct timespec *at)
{
    int e = pthread_cond_timedwait(cond, mtx, at);
    if (e == 0)
        return thrd_success;
    return e == ETIMEDOUT ? thrd_timedout : thrd_error;
}

void cnd_destroy(cnd_t *cond)
{
    pthread_cond_destroy(cond);
}

void *tss_get(tss_t key)
{
    return pthread_getspecific(key);
}

int tss_set(tss_t key, void *val)
{
    return pthread_setspecific(key, val) == 0 ? thrd_success : thrd_error;
}

int tss_create(tss_t *key, void (*dtor)(void *))
{
    return pthread_key_create(key, dtor) == 0 ? thrd_success : thrd_error;
}

void tss_delete(tss_t key)
{
    pthread_key_delete(key);
}

void call_once(once_flag *flag, void (*func)(void))
{
    pthread_once(flag, func);
}
