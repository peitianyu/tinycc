/* portable/port.c — complete POSIX polyfill implementation for Windows
 *
 * Link:
 *   gcc -Iportable myapp.c portable/port.c -o myapp.exe -lws2_32
 *   tcc -Iportable myapp.c portable/port.c -o myapp.exe
 *
 * Modules implemented:
 *   termios, sys/ioctl, sys/select, signal    (original)
 *   dlfcn, pthread, sys/mman, poll, glob      (new)
 *   unistd (getline, usleep, getopt, ...)     (new)
 *   string_ext (strdup, strndup, ...)          (new)
 *   stdio_ext (asprintf)                       (new)
 *   dirent (scandir)                           (new)
 *   sys/time (gettimeofday)                    (new)
 */

#ifndef NO_PORTABLE_POLYFILL

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <windows.h>
#include <io.h>
#include <direct.h>
#include <process.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <malloc.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>

/* prototypes suppressed by __NO_ISOCEXT */
#ifndef strnlen
size_t strnlen(const char *s, size_t maxlen);
#endif
#ifndef strerror_s
int strerror_s(char *buf, size_t buflen, int errnum);
#endif
#ifndef _aligned_malloc
void *_aligned_malloc(size_t size, size_t alignment);
void _aligned_free(void *p);
#endif

/* === our headers (after windows.h for type compatibility) === */

#ifdef TCC_PORTABLE_EMBEDDED
# include <portable/termios.h>
# include <portable/sys/ioctl.h>
# include <portable/sys/select.h>
# include <portable/signal.h>
# include <portable/pthread.h>
# include <portable/unistd.h>
# include <portable/dlfcn.h>
# include <portable/sys/mman.h>
# include <portable/poll.h>
# include <portable/glob.h>
# include <portable/dirent.h>
# include <portable/sys/time.h>
# include <portable/time.h>
# include <portable/stdlib_ext.h>
# include <portable/string_ext.h>
# include <portable/stdio_ext.h>
# include <portable/syslog.h>
#else
# include "termios.h"
# include "sys/ioctl.h"
# include "sys/select.h"
# include "signal.h"
# include "pthread.h"
# include "unistd.h"
# include "dlfcn.h"
# include "sys/mman.h"
# include "poll.h"
# include "glob.h"
# include "dirent.h"
# include "sys/time.h"
# include "time.h"
# include "stdlib.h"
# include "string.h"
# include "stdio.h"
# include "syslog.h"
#endif

#ifndef ENABLE_VIRTUAL_TERMINAL_INPUT
#define ENABLE_VIRTUAL_TERMINAL_INPUT 0x0200
#endif
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/* === helpers === */
static HANDLE f2h(int fd) {
    if (fd == 0) return GetStdHandle(STD_INPUT_HANDLE);
    if (fd == 1) return GetStdHandle(STD_OUTPUT_HANDLE);
    if (fd == 2) return GetStdHandle(STD_ERROR_HANDLE);
    return (HANDLE)(INT_PTR)_get_osfhandle(fd);
}

/* =================================================================
 *  1.  TERMIOS  (tcgetattr, tcsetattr, cfmakeraw, tcflush)
 * ================================================================= */
int tcgetattr(int fd, struct termios *t) {
    HANDLE h; DWORD m;
    if (!t) { errno = EFAULT; return -1; }
    memset(t, 0, sizeof(*t));
    h = f2h(fd);
    if (h == INVALID_HANDLE_VALUE) { errno = EBADF; return -1; }
    if (fd == 0) {
        if (!GetConsoleMode(h, &m)) { errno = ENOTTY; return -1; }
        if (!(m & ENABLE_LINE_INPUT))  t->c_lflag &= ~ICANON; else t->c_lflag |= ICANON;
        if (!(m & ENABLE_ECHO_INPUT))  t->c_lflag &= ~ECHO;   else t->c_lflag |= ECHO;
        if  (m & ENABLE_PROCESSED_INPUT) t->c_lflag |= ISIG;
        t->c_cc[VMIN]=1; t->c_cc[VTIME]=0; t->c_cc[VEOF]=4;
        return 0;
    }
    if (fd == 1 || fd == 2) {
        if (!GetConsoleMode(h, &m)) { errno = ENOTTY; return -1; }
        t->c_oflag |= OPOST; return 0;
    }
    t->c_cc[VMIN]=1; t->c_cc[VTIME]=0; return 0;
}
int tcsetattr(int fd, int action, const struct termios *t) {
    HANDLE h; DWORD m; (void)action;
    if (!t) { errno = EFAULT; return -1; }
    h = f2h(fd); if (h == INVALID_HANDLE_VALUE) { errno = EBADF; return -1; }
    if (fd == 0) {
        if (!GetConsoleMode(h, &m)) { errno = ENOTTY; return -1; }
        m &= ~(ENABLE_LINE_INPUT|ENABLE_ECHO_INPUT|ENABLE_WINDOW_INPUT|ENABLE_MOUSE_INPUT);
        m |= ENABLE_PROCESSED_INPUT;
        if (t->c_lflag & ICANON) m |= ENABLE_LINE_INPUT;
        if (t->c_lflag & ECHO)   m |= ENABLE_ECHO_INPUT;
        m |= ENABLE_VIRTUAL_TERMINAL_INPUT;
        if (!SetConsoleMode(h, m)) { errno = EINVAL; return -1; }
        return 0;
    }
    if (fd == 1 || fd == 2) {
        if (!GetConsoleMode(h, &m)) { errno = ENOTTY; return -1; }
        m |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        if (!SetConsoleMode(h, m)) { errno = EINVAL; return -1; }
        return 0;
    }
    return 0;
}
void cfmakeraw(struct termios *t) {
    t->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
    t->c_oflag &= ~OPOST;
    t->c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
    t->c_cflag &= ~(CSIZE|PARENB); t->c_cflag |= CS8;
    t->c_cc[VMIN]=1; t->c_cc[VTIME]=0;
}
int tcflush(int fd, int qs) {
    (void)qs; HANDLE h = f2h(fd);
    if (h == INVALID_HANDLE_VALUE) { errno = EBADF; return -1; }
    if (fd == 0) FlushConsoleInputBuffer(h);
    return 0;
}
speed_t cfgetispeed(const struct termios *t) { (void)t; return B115200; }
speed_t cfgetospeed(const struct termios *t) { (void)t; return B115200; }
int cfsetispeed(struct termios *t, speed_t s) { (void)t; (void)s; return 0; }
int cfsetospeed(struct termios *t, speed_t s) { (void)t; (void)s; return 0; }

/* =================================================================
 *  2.  FD_SET helpers / SELECT  for console
 * ================================================================= */
void FD_ZERO_(fd_set *s) { s->fd_count = 0; }
void FD_SET_(int fd, fd_set *s) {
    if (s->fd_count < FD_SETSIZE) s->fd_array[s->fd_count++] = (unsigned)fd;
}
void FD_CLR_(int fd, fd_set *s) {
    unsigned i; for (i=0;i<s->fd_count;i++)
        if (s->fd_array[i]==(unsigned)fd) {
            while (++i<s->fd_count) s->fd_array[i-1]=s->fd_array[i];
            s->fd_count--; return;
        }
}
int FD_ISSET_(int fd, fd_set *s) {
    unsigned i; for (i=0;i<s->fd_count;i++) if (s->fd_array[i]==(unsigned)fd) return 1;
    return 0;
}

int __port_select(int nfds, fd_set *rfds, fd_set *wfds, fd_set *efds, struct timeval *tv) {
    HANDLE handles[64]; int h2f[64], nh=0, i; DWORD ms;
    (void)nfds; (void)wfds; (void)efds;
    if (rfds) {
        fd_set result; int rc=0; FD_ZERO(&result);
        for (i=0;i<(int)rfds->fd_count;i++) {
            int fd=(int)rfds->fd_array[i]; HANDLE h=f2h(fd);
            if (h==INVALID_HANDLE_VALUE) continue;
            if (fd==0) { INPUT_RECORD ir; DWORD nr;
                if (PeekConsoleInputA(h,&ir,1,&nr)&&nr>0) {FD_SET(fd,&result);rc++;continue;} }
            handles[nh]=h; h2f[nh]=fd; nh++;
        }
        if (rc>0) { memcpy(rfds,&result,sizeof(fd_set)); return rc; }
        ms=tv?(DWORD)(tv->tv_sec*1000+tv->tv_usec/1000):INFINITE;
        if (nh==0) { if (tv&&ms>0) Sleep(ms); FD_ZERO(rfds); return 0; }
        DWORD ret=WaitForMultipleObjects(nh,handles,FALSE,ms);
        if (ret>=WAIT_OBJECT_0&&ret<WAIT_OBJECT_0+(DWORD)nh) {
            int idx=(int)(ret-WAIT_OBJECT_0); FD_ZERO(rfds); FD_SET(h2f[idx],rfds); return 1; }
        if (ret==WAIT_TIMEOUT) { FD_ZERO(rfds); return 0; }
        errno=EINTR; return -1;
    }
    if (tv) { ms=(DWORD)(tv->tv_sec*1000+tv->tv_usec/1000); if (ms>0) Sleep(ms); }
    return 0;
}

/* =================================================================
 *  3.  IOCTL (TIOCGWINSZ)
 * ================================================================= */
int ioctl(int fd, unsigned long req, ...) {
    HANDLE h; CONSOLE_SCREEN_BUFFER_INFO bi;
    void *ap; va_list vl; va_start(vl, req); ap=va_arg(vl,void*); va_end(vl);
    if (req==TIOCGWINSZ) {
        struct winsize *w=(struct winsize*)ap;
        if (!w) { errno=EFAULT; return -1; }
        h=f2h(fd>=0?fd:1);
        if (h==INVALID_HANDLE_VALUE) { errno=EBADF; return -1; }
        memset(w,0,sizeof(*w));
        if (GetConsoleScreenBufferInfo(h,&bi)) {
            w->ws_col=(unsigned short)(bi.srWindow.Right-bi.srWindow.Left+1);
            w->ws_row=(unsigned short)(bi.srWindow.Bottom-bi.srWindow.Top+1);
        } else { w->ws_col=80; w->ws_row=25; }
        return 0;
    }
    errno=EINVAL; return -1;
}

/* =================================================================
 *  4.  SIGACTION / SIGWINCH
 * ================================================================= */
#define _SIGWINCH 28
static void (*_sw_handler)(int)=NULL;
static BOOL WINAPI _ctrl_h(DWORD t) { (void)t; if (_sw_handler) _sw_handler(_SIGWINCH); return TRUE; }
/* struct sigaction is defined in portable/signal.h */

int sigaction(int sig, const struct sigaction *act, struct sigaction *old) {
    if (sig==_SIGWINCH) {
        if (old) { old->sa_handler=_sw_handler; old->sa_flags=0; }
        if (act) {
            _sw_handler=act->sa_handler;
            SetConsoleCtrlHandler(_ctrl_h, act->sa_handler!=SIG_DFL&&act->sa_handler!=SIG_IGN);
        }
        return 0;
    }
    if (old) { void(*o)(int)=signal(sig,act?act->sa_handler:SIG_DFL); if(o==SIG_ERR)return -1; old->sa_handler=o;old->sa_flags=0; }
    else if(act) { if(signal(sig,act->sa_handler)==SIG_ERR)return -1; }
    return 0;
}

/* =================================================================
 *  5.  DLFCN  (dlopen, dlsym, dlclose, dlerror)
 * ================================================================= */
void *dlopen(const char *fn, int flags) {
    (void)flags;
    return (void*)LoadLibraryA(fn);
}
void *dlsym(void *h, const char *sym) {
    return (void*)GetProcAddress((HMODULE)h, sym);
}
int dlclose(void *h) {
    return FreeLibrary((HMODULE)h) ? 0 : -1;
}
char *dlerror(void) {
    static char buf[256];
    DWORD e = GetLastError();
    if (e == 0) return NULL;
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, e, 0, buf, sizeof(buf), NULL);
    return buf;
}

/* =================================================================
 *  6.  PTHREAD  (simplified)
 * ================================================================= */
typedef struct { void *(*fn)(void*); void *arg; } thunk_t;
static DWORD WINAPI _thread_thunk(LPVOID p) {
    thunk_t *t = (thunk_t*)p;
    void *(*fn)(void*) = t->fn;
    void *arg = t->arg;
    free(t);
    return (DWORD)(INT_PTR)fn(arg);
}
int pthread_create(pthread_t *th, const void *attr, void *(*fn)(void*), void *arg) {
    (void)attr; thunk_t *t = (thunk_t*)malloc(sizeof(thunk_t));
    if (!t) return -1;
    t->fn = fn; t->arg = arg;
    *th = CreateThread(NULL, 0, _thread_thunk, t, 0, NULL);
    return *th ? 0 : -1;
}
int pthread_join(pthread_t th, void **ret) {
    WaitForSingleObject(th, INFINITE);
    if (ret) { DWORD r; GetExitCodeThread(th, &r); *ret = (void*)(INT_PTR)r; }
    CloseHandle(th); return 0;
}
int pthread_mutex_init(pthread_mutex_t *m, const void *a) { (void)a; InitializeCriticalSection((CRITICAL_SECTION*)m); return 0; }
int pthread_mutex_lock(pthread_mutex_t *m) { EnterCriticalSection((CRITICAL_SECTION*)m); return 0; }
int pthread_mutex_unlock(pthread_mutex_t *m) { LeaveCriticalSection((CRITICAL_SECTION*)m); return 0; }
int pthread_mutex_destroy(pthread_mutex_t *m) { DeleteCriticalSection((CRITICAL_SECTION*)m); return 0; }

int sem_init(sem_t *s, int p, unsigned v) { (void)p; *s = CreateSemaphoreA(NULL, v, 0x7FFFFFFF, NULL); return *s ? 0 : -1; }
int sem_wait(sem_t *s) { return WaitForSingleObject(*s, INFINITE)==WAIT_OBJECT_0 ? 0 : -1; }
int sem_post(sem_t *s) { return ReleaseSemaphore(*s, 1, NULL) ? 0 : -1; }
int sem_destroy(sem_t *s) { CloseHandle(*s); return 0; }

/* =================================================================
 *  7.  SYS/MMAN  (mmap, munmap)
 * ================================================================= */
void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off) {
    DWORD flProtect = 0, flAlloc = MEM_RESERVE|MEM_COMMIT;
    (void)fd; (void)off; /* anonymous only */
    if (!(flags & MAP_ANONYMOUS)) { fprintf(stderr,"mmap: file-backed not supported\n"); errno=ENODEV; return MAP_FAILED; }
    if (prot & PROT_WRITE) flProtect = PAGE_READWRITE;
    else if (prot & PROT_READ) flProtect = PAGE_READONLY;
    else flProtect = PAGE_NOACCESS;
    if (prot & PROT_EXEC) flProtect = PAGE_EXECUTE_READWRITE;
    void *p = VirtualAlloc(addr, len, flAlloc, flProtect);
    return p ? p : MAP_FAILED;
}
int munmap(void *addr, size_t len) {
    (void)len;
    return VirtualFree(addr, 0, MEM_RELEASE) ? 0 : -1;
}

/* =================================================================
 *  8.  POLL
 * ================================================================= */
int poll(struct pollfd *fds, nfds_t n, int ms) {
    HANDLE handles[64]; int h2i[64], nh=0, ready=0;
    for (nfds_t i=0; i<n; i++) { fds[i].revents = 0; }
    for (nfds_t i=0; i<n; i++) {
        if (fds[i].fd < 0) continue;
        HANDLE h = f2h(fds[i].fd);
        if (h != INVALID_HANDLE_VALUE && (fds[i].events & POLLIN)) {
            if (fds[i].fd==0) { INPUT_RECORD ir; DWORD nr;
                if (PeekConsoleInputA(h,&ir,1,&nr)&&nr>0) { fds[i].revents=POLLIN; ready++; continue; } }
            handles[nh]=h; h2i[nh]=(int)i; nh++;
        }
    }
    if (ready>0) return ready;
    if (nh==0) { if (ms>0) Sleep((DWORD)ms); return 0; }
    DWORD ret = WaitForMultipleObjects(nh, handles, FALSE, ms<0?INFINITE:(DWORD)ms);
    if (ret>=WAIT_OBJECT_0 && ret<WAIT_OBJECT_0+(DWORD)nh) {
        int idx=h2i[ret-WAIT_OBJECT_0]; fds[idx].revents=POLLIN; return 1; }
    if (ret==WAIT_TIMEOUT) return 0;
    errno=EINTR; return -1;
}

/* =================================================================
 *  9.  GLOB
 * ================================================================= */
int glob(const char *pat, int flags, int (*errf)(const char*,int), glob_t *g) {
    WIN32_FIND_DATAA fd; HANDLE h; int n=0, cap=16; char dir[1024]=".";
    (void)errf; (void)flags;
    memset(g,0,sizeof(*g));
    g->gl_pathv = (char**)malloc(cap * sizeof(char*));
    if (!g->gl_pathv) return GLOB_NOMATCH;
    h = FindFirstFileA(pat, &fd);
    if (h == INVALID_HANDLE_VALUE) { free(g->gl_pathv); g->gl_pathv=NULL; return GLOB_NOMATCH; }
    do {
        if (n >= cap) { cap*=2; g->gl_pathv=(char**)realloc(g->gl_pathv,cap*sizeof(char*)); }
        g->gl_pathv[n++] = strdup(fd.cFileName);
    } while (FindNextFileA(h, &fd));
    FindClose(h);
    g->gl_pathc = n;
    return 0;
}
void globfree(glob_t *g) {
    if (g->gl_pathv) { for (size_t i=0; i<g->gl_pathc; i++) free(g->gl_pathv[i]); free(g->gl_pathv); }
    memset(g,0,sizeof(*g));
}

/* =================================================================
 *  10.  UNISTD  (getpid, usleep, getline, getopt, pread, pwrite, fsync)
 * ================================================================= */
int getpid(void) { return (int)GetCurrentProcessId(); }
unsigned int sleep(unsigned int s) { Sleep(s*1000); return 0; }
int usleep(unsigned int us) { Sleep(us/1000 + (us%1000?1:0)); return 0; }

ssize_t getline(char **p, size_t *n, FILE *stream) {
    if (!p || !n || !stream) { errno=EINVAL; return -1; }
    if (!*p) { *n = 256; *p = (char*)malloc(*n); if (!*p) return -1; }
    size_t i = 0; int c;
    while ((c = fgetc(stream)) != EOF) {
        if (i+1 >= *n) { *n *= 2; char *t = (char*)realloc(*p, *n); if (!t) return -1; *p = t; }
        (*p)[i++] = (char)c;
        if (c == '\n') break;
    }
    if (i==0 && c==EOF) return -1;
    (*p)[i] = '\0';
    return (ssize_t)i;
}

ssize_t pread(int fd, void *buf, size_t count, off_t offset) {
    HANDLE h = f2h(fd);
    if (h == INVALID_HANDLE_VALUE) { errno = EBADF; return -1; }
    DWORD n; OVERLAPPED ov = {0};
    ov.Offset = (DWORD)offset;
    ov.OffsetHigh = (DWORD)((uint64_t)offset >> 32);
    if (!ReadFile(h, buf, (DWORD)count, &n, &ov)) return -1;
    return (ssize_t)n;
}
ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset) {
    HANDLE h = f2h(fd);
    if (h == INVALID_HANDLE_VALUE) { errno = EBADF; return -1; }
    DWORD n; OVERLAPPED ov = {0};
    ov.Offset = (DWORD)offset;
    ov.OffsetHigh = (DWORD)((uint64_t)offset >> 32);
    if (!WriteFile(h, buf, (DWORD)count, &n, &ov)) return -1;
    return (ssize_t)n;
}
int fsync(int fd) {
    HANDLE h = f2h(fd);
    if (h == INVALID_HANDLE_VALUE) { errno = EBADF; return -1; }
    return FlushFileBuffers(h) ? 0 : -1;
}

/* getopt (simplified) */
char *optarg = NULL; int optind = 1, opterr = 1, optopt = 0;
int getopt(int argc, char *const argv[], const char *os) {
    static int pos = 0;
    if (optind >= argc || !argv[optind] || argv[optind][0]!='-' || argv[optind][1]=='\0') return -1;
    if (!strcmp(argv[optind],"--")) { optind++; return -1; }
    char c = argv[optind][++pos];
    if (!c) { optind++; pos=0; return -1; }
    const char *p = strchr(os, c);
    if (!p) { optopt = c; if (opterr) fprintf(stderr,"%s: illegal option -- %c\n", argv[0], c); return '?'; }
    if (p[1]==':') {
        if (argv[optind][pos+1]) { optarg = argv[optind]+pos+1; }
        else { optind++; if (optind>=argc) { optopt=c; if (opterr) fprintf(stderr,"%s: option requires argument -- %c\n",argv[0],c); return '?'; } optarg=argv[optind]; }
        optind++; pos=0;
    }
    return c;
}

/* =================================================================
 *  11.  STRING extensions
 * ================================================================= */
char *strdup(const char *s) { if(!s)return NULL; size_t n=strlen(s)+1; char *d=(char*)malloc(n); return d?memcpy(d,s,n):NULL; }
char *strndup(const char *s, size_t n) {
    if(!s)return NULL; size_t len=strnlen(s,n); char *d=(char*)malloc(len+1);
    if(!d)return NULL; memcpy(d,s,len); d[len]='\0'; return d;
}
char *strsep(char **sp, const char *delim) {
    if(!sp||!*sp)return NULL; char *s=*sp,*e=s+strcspn(s,delim);
    if(*e) *e++='\0'; else e=NULL; *sp=e; return s;
}
void *memrchr(const void *s, int c, size_t n) {
    const unsigned char *p=(const unsigned char*)s; while(n--) if(p[n]==(unsigned char)c) return (void*)(p+n);
    return NULL;
}
char *strcasestr(const char *hay, const char *ndl) {
    if(!*ndl)return(char*)hay;
    size_t nl=strlen(ndl);
    for(;*hay;hay++) { size_t i; for(i=0;i<nl;i++) if(tolower((unsigned char)hay[i])!=tolower((unsigned char)ndl[i])) break;
        if(i==nl)return(char*)hay; }
    return NULL;
}
int strerror_r(int en, char *buf, size_t blen) {
    return strerror_s(buf, blen, en) ? -1 : 0;
}

/* =================================================================
 *  12.  STDIO extensions (asprintf, vasprintf)
 * ================================================================= */
int vasprintf(char **sp, const char *fmt, va_list ap) {
    int n = _vscprintf(fmt, ap);
    if (n < 0) return -1;
    *sp = (char*)malloc(n + 1);
    if (!*sp) return -1;
    return vsprintf(*sp, fmt, ap);
}
int asprintf(char **sp, const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    int n = vasprintf(sp, fmt, ap);
    va_end(ap); return n;
}

/* =================================================================
 *  13.  DIRENT extensions (scandir, alphasort)
 * ================================================================= */
int alphasort(const struct dirent **a, const struct dirent **b) {
    return strcmp((*a)->d_name, (*b)->d_name);
}
int scandir(const char *dir, struct dirent ***nl,
            int (*filter)(const struct dirent *),
            int (*compar)(const struct dirent **, const struct dirent **))
{
    WIN32_FIND_DATAA fd; char pat[2048];
    _snprintf(pat,sizeof(pat),"%s\\*", dir);
    HANDLE h = FindFirstFileA(pat, &fd);
    if (h == INVALID_HANDLE_VALUE) return -1;
    int n = 0, cap = 16;
    struct dirent **list = (struct dirent**)malloc(cap * sizeof(struct dirent*));
    if (!list) { FindClose(h); return -1; }
    do {
        struct dirent *d = (struct dirent*)malloc(sizeof(struct dirent));
        if (!d) break;
        strncpy(d->d_name, fd.cFileName, sizeof(d->d_name)-1);
        d->d_name[sizeof(d->d_name)-1] = '\0';
        if (!filter || filter(d)) {
            if (n >= cap) { cap*=2; list=(struct dirent**)realloc(list,cap*sizeof(struct dirent*)); }
            list[n++] = d;
        } else free(d);
    } while (FindNextFileA(h, &fd));
    FindClose(h);
    if (compar) qsort(list, n, sizeof(struct dirent*),
        (int(*)(const void*,const void*))compar);
    *nl = list;
    return n;
}

/* =================================================================
 *  14.  GETTIMEOFDAY
 * ================================================================= */
#ifndef _GETTIMEOFDAY_DEFINED
int gettimeofday(struct timeval *__restrict__ tv, void *__restrict__ tz) {
    (void)tz;
    if (!tv) return 0;
    static LARGE_INTEGER freq = {0};
    LARGE_INTEGER now;
    if (freq.QuadPart == 0) { QueryPerformanceFrequency(&freq);
        if (freq.QuadPart == 0) freq.QuadPart = 10000000; }
    QueryPerformanceCounter(&now);
    tv->tv_sec  = (long)(now.QuadPart / freq.QuadPart);
    tv->tv_usec = (long)(((now.QuadPart % freq.QuadPart) * 1000000) / freq.QuadPart);
    return 0;
}
#endif

/* =================================================================
 *  15.  STUBS — fork, exec, epoll (print warnings)
 * ================================================================= */
pid_t fork(void) {
    fprintf(stderr, "WARNING: fork() is not supported on Windows — returning -1\n");
    errno = ENOSYS; return -1;
}
/* execvp/execv: already provided by MSVCRT <process.h>. Not redefined here. */
int epoll_create(int size) {
    (void)size;
    fprintf(stderr, "WARNING: epoll is Linux-specific; use poll() instead — returning -1\n");
    errno = ENOSYS; return -1;
}
int epoll_wait(int epfd, void *events, int maxevents, int timeout) {
    (void)epfd; (void)events; (void)maxevents; (void)timeout;
    fprintf(stderr, "WARNING: epoll is Linux-specific; use poll() instead — returning -1\n");
    errno = ENOSYS; return -1;
}

/* =================================================================
 *  16.  STDLIB extensions (setenv, unsetenv, realpath, mkstemp, posix_memalign, random/srandom)
 * ================================================================= */
int setenv(const char *name, const char *value, int overwrite) {
    if (!overwrite && getenv(name)) return 0;
    return _putenv_s(name, value ? value : "");
}
int unsetenv(const char *name) { return _putenv_s(name, ""); }

char *realpath(const char *path, char *resolved) {
    if (!resolved) resolved = (char*)malloc(PATH_MAX);
    if (!resolved) return NULL;
    if (!GetFullPathNameA(path, PATH_MAX, resolved, NULL)) { free(resolved); return NULL; }
    return resolved;
}

int mkstemp(char *templ) {
    char *p = _mktemp(templ);
    if (!p) return -1;
    return _open(p, _O_CREAT|_O_RDWR|_O_BINARY|_O_TEMPORARY, _S_IREAD|_S_IWRITE);
}

int posix_memalign(void **memptr, size_t alignment, size_t size) {
    /* _aligned_malloc: must use _aligned_free(), NOT free().
     * Use posix_memalign_free(p) macro from stdlib_ext.h. */
    if (alignment < sizeof(void*)) alignment = sizeof(void*);
    if (alignment & (alignment-1)) return EINVAL;
    *memptr = _aligned_malloc(size, alignment);
    return *memptr ? 0 : ENOMEM;
}
void posix_memalign_free(void *p) { if (p) _aligned_free(p); }

static unsigned long _rng_state = 1;
long random(void) {
    _rng_state = _rng_state * 1103515245 + 12345;
    return (long)((_rng_state >> 16) & 0x7FFFFFFF);
}
void srandom(unsigned int seed) { _rng_state = seed; }

/* =================================================================
 *  17.  TIME extensions (nanosleep, clock_gettime)
 *  NOTE: skip for embedded GCC build — MinGW winpthread provides these
 * ================================================================= */
#ifndef TCC_PORTABLE_EMBEDDED
int nanosleep(const struct timespec *req, struct timespec *rem) {
    if (rem) { rem->tv_sec = 0; rem->tv_nsec = 0; }
    DWORD ms = (DWORD)(req->tv_sec * 1000 + req->tv_nsec / 1000000);
    if (ms == 0 && req->tv_nsec > 0) ms = 1;
    Sleep(ms);
    return 0;
}
int clock_gettime(int clk_id, struct timespec *tp) {
    static LARGE_INTEGER freq = {0};
    LARGE_INTEGER now;
    (void)clk_id;
    if (freq.QuadPart == 0) { QueryPerformanceFrequency(&freq); if (freq.QuadPart==0) freq.QuadPart=10000000; }
    QueryPerformanceCounter(&now);
    tp->tv_sec  = (long)(now.QuadPart / freq.QuadPart);
    tp->tv_nsec = (long)(((now.QuadPart % freq.QuadPart) * 1000000000LL) / freq.QuadPart);
    return 0;
}
#endif

/* =================================================================
 *  18.  UNISTD additions (ftruncate, symlink, readlink, link, chown)
 * ================================================================= */
int ftruncate(int fd, off_t length) {
    HANDLE h = f2h(fd);
    if (h == INVALID_HANDLE_VALUE) { errno = EBADF; return -1; }
    LARGE_INTEGER li; li.QuadPart = length;
    if (!SetFilePointerEx(h, li, NULL, FILE_BEGIN)) { errno = EINVAL; return -1; }
    return SetEndOfFile(h) ? 0 : -1;
}
#ifndef SYMBOLIC_LINK_FLAG_DIRECTORY
#define SYMBOLIC_LINK_FLAG_DIRECTORY 0x1
#endif
typedef BOOLEAN (WINAPI *CSLA_t)(LPCSTR, LPCSTR, DWORD);
int symlink(const char *target, const char *linkpath) {
    static CSLA_t pCSLA = NULL;
    if (!pCSLA) {
        HMODULE k = GetModuleHandleA("kernel32.dll");
        pCSLA = (CSLA_t)GetProcAddress(k, "CreateSymbolicLinkA");
    }
    if (!pCSLA) {
        fprintf(stderr, "WARNING: symlink() not available — returning -1\n");
        errno = ENOSYS; return -1;
    }
    DWORD flags = (GetFileAttributesA(target) & FILE_ATTRIBUTE_DIRECTORY)
        ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0;
    if (pCSLA(linkpath, target, flags)) return 0;
    DWORD e = GetLastError();
    if (e == ERROR_PRIVILEGE_NOT_HELD)
        fprintf(stderr, "WARNING: symlink() requires admin or Developer Mode — returning -1\n");
    else
        fprintf(stderr, "WARNING: symlink() failed (err=%lu) — returning -1\n", e);
    errno = (e == ERROR_PRIVILEGE_NOT_HELD) ? EACCES : ENOSYS;
    return -1;
}
ssize_t readlink(const char *pathname, char *buf, size_t bufsiz) {
    (void)pathname; (void)buf; (void)bufsiz;
    fprintf(stderr, "WARNING: readlink() is not supported on Windows — returning -1\n");
    errno = ENOSYS; return -1;
}
typedef BOOL (WINAPI *CHLA_t)(LPCSTR, LPCSTR, LPSECURITY_ATTRIBUTES);
int link(const char *oldpath, const char *newpath) {
    static CHLA_t pCHLA = NULL;
    if (!pCHLA) {
        HMODULE k = GetModuleHandleA("kernel32.dll");
        pCHLA = (CHLA_t)GetProcAddress(k, "CreateHardLinkA");
    }
    if (!pCHLA) {
        fprintf(stderr, "WARNING: link() not available — returning -1\n");
        errno = ENOSYS; return -1;
    }
    if (pCHLA(newpath, oldpath, NULL)) return 0;
    fprintf(stderr, "WARNING: link() requires NTFS — returning -1\n");
    errno = ENOSYS; return -1;
}
int chown(const char *pathname, unsigned int owner, unsigned int group) {
    (void)pathname; (void)owner; (void)group;
    /* chown is meaningless on Windows */
    return 0;
}

/* =================================================================
 *  19.  SYSLOG stubs
 * ================================================================= */
void openlog(const char *ident, int option, int facility) {
    (void)ident; (void)option; (void)facility;
    /* no-op on Windows */
}
void syslog(int priority, const char *format, ...) {
    (void)priority;
    va_list ap; va_start(ap, format);
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
    va_end(ap);
}
void closelog(void) { /* no-op */ }

#endif /* NO_PORTABLE_POLYFILL */
