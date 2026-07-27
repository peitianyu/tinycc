/* portable/pthread.h — simplified POSIX threads for Windows
 * Include with: -Iportable
 * Usage: #include <pthread.h>  (standard, unchanged)
 *
 * Covers: pthread_create, pthread_join, pthread_mutex, sem_*
 * Not covered: pthread_cond (use CONDITION_VARIABLE directly),
 *              pthread_attr, pthread_key, pthread_rwlock.
 */

#ifndef _PORTABLE_PTHREAD_H
#define _PORTABLE_PTHREAD_H

#ifdef __cplusplus
extern "C" {
#endif

/* avoid <windows.h> to prevent 0.9.28rc intrinsic link errors */
typedef void *pthread_t;

/* RTL_CRITICAL_SECTION is sizeof(void*) + 4*long on x86_64 (=40 bytes) */
typedef struct { void *dummy[5]; } pthread_mutex_t;

#define PTHREAD_MUTEX_INITIALIZER {0}

int pthread_create(pthread_t *thread, const void *attr,
                   void *(*start_routine)(void*), void *arg);
int pthread_join(pthread_t thread, void **retval);
int pthread_mutex_init(pthread_mutex_t *mutex, const void *attr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_destroy(pthread_mutex_t *mutex);

/* semaphore */
typedef void *sem_t;
int sem_init(sem_t *sem, int pshared, unsigned int value);
int sem_wait(sem_t *sem);
int sem_post(sem_t *sem);
int sem_destroy(sem_t *sem);

#ifdef __cplusplus
}
#endif

#endif
