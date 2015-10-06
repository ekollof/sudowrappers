/*
 * Copyright (c) 2011, Emiel Kollof <coolvibe@hackerheaven.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY EMIEL KOLLOF AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL EMIEL KOLLOF AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This is libwrapper, part of the sudowrapper suite by
 * Emiel Kollof <coolvibe@hackerheaven.org>.
 *
 * This library wraps around some fairly common system and library calls
 * to stop any shenanigans.
 *
 * The following calls will be wrapped:
 * fopen, open/open64/openat, kill, signal, unlink/unlinkat, rename,
 * setenv/putenv, execve
 *
 */

#define _GNU_SOURCE
#include <sys/param.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>

extern int errno;

/* XXX: Find a neater way to do this... */
#ifdef FREEBSD
typedef void (*sighandler_t) (int);
#endif

/* prototypes */
int siginlist(int, const char *);
int fileinpath(const char *, const char *);
void debug(char *, ...);


/* make sure these correspond to what's in the respective manual pages. */
int (*sys_unlink)(const char *);
int (*sys_open)(const char *, int, unsigned short);
int (*sys_open64)(const char *, int, unsigned short);
FILE *(*sys_fopen)(const char *pathname, const char *mode);
int (*sys_rename)(const char *, const char *);
sighandler_t (*sys_signal)(int, sighandler_t);
int (*sys_kill)(pid_t, int);
int (*sys_unlinkat)(int dirfd, const char *pathname, int flags);
int (*sys_setenv)(const char *name, const char *value, int overwrite);
int (*sys_unsetenv)(const char *name);
int (*sys_putenv)(char *string);
int (*sys_execve)(const char *filename, char *const argv[],
        char *const envp[]);

void _init(void) {
    /* Yay, clash between ISO C and SUSv3, although this
     * very dirty approach seems to shut up the compiler
     * about it.
     */
    *(void **) (&sys_unlink) = dlsym(RTLD_NEXT, "unlink");
    *(void **) (&sys_open) = dlsym(RTLD_NEXT, "open");
    *(void **) (&sys_open64) = dlsym(RTLD_NEXT, "open64");
    *(void **) (&sys_fopen) = dlsym(RTLD_NEXT, "fopen");
    *(void **) (&sys_rename) = dlsym(RTLD_NEXT, "rename");
    *(void **) (&sys_signal) = dlsym(RTLD_NEXT, "signal");
    *(void **) (&sys_kill) = dlsym(RTLD_NEXT, "kill");
    *(void **) (&sys_unlinkat) = dlsym(RTLD_NEXT, "unlinkat");
    *(void **) (&sys_setenv) = dlsym(RTLD_NEXT, "setenv");
    *(void **) (&sys_unsetenv) = dlsym(RTLD_NEXT, "unsetenv");
    *(void **) (&sys_putenv) = dlsym(RTLD_NEXT, "putenv");
    *(void **) (&sys_execve) = dlsym(RTLD_NEXT, "execve");
}

int execve(const char *filename, char *const argv[],
        char *const envp[]) {
    if (getenv("EXEC_ALLOWED")) {
        return sys_execve(filename, argv, envp);
    }
    debug("execve: ");
    errno = EPERM;
    return -1;
}


int putenv(char *string) {
    if (getenv("PUTENV_ALLOWED")) {
        return sys_putenv(string);
    }

    debug("putenv: ");
    errno = ENOMEM;
    return -1;
}

int setenv(const char *name, const char *value, int overwrite) {

    if (fileinpath(name, "SUDO_TOUCHENV") ||
            getenv("SYS_SETENV")) {
        return sys_setenv(name, value, overwrite);
    }
    debug("setenv: ");
    errno = EINVAL;
    return -1;
}

int unsetenv(const char *name) {

    if (fileinpath(name, "SUDO_TOUCHENV") ||
            getenv("SYS_SETENV")) {
        return sys_unsetenv(name);
    }

    debug("unsetenv: ");
    errno = EINVAL;
    return -1;
}


int unlinkat(int dirfd, const char *pathname, int flags) {
    if (fileinpath(pathname, "SUDO_ALLOWED") ||
            getenv("SYS_UNLINK")) {

        return sys_unlinkat(dirfd, pathname, flags);
    }

    debug( "unlinkat: ");
    errno = EPERM;
    return -1;
}

int kill(pid_t pid, int sig) {

    debug("Sending %d to pid %d\n", sig, pid);

    if (siginlist(sig, "SUDO_SIGLIST") ||
            getenv("SYS_SIGNAL")) {

        return sys_kill(pid, sig);
    }
    debug("kill: ");
    errno = EPERM;
    return -1;
}

/* NOTE: This is for signal handlers, not for allowing/disallowing signals,
 * use kill(2) for that
 */
    sighandler_t signal(int signum, sighandler_t handler) {

        if (siginlist(signum, "SUDO_SIGLIST") ||
                getenv("SYS_SIGNAL")) {

            return sys_signal(signum, handler);
        }
        debug("signal: ");
        errno = EINVAL;
        return SIG_ERR;
    }

int rename(const char *oldpath, const char *newpath) {

    if (fileinpath(oldpath, "SUDO_ALLOWED") ||
            fileinpath(newpath, "SUDO_ALLOWED") ||
            getenv("SYS_RENAME")) {

        return sys_rename(oldpath, newpath);
    }

    debug( "rename: ");
    errno = EPERM;
    return -1;
}

FILE *fopen(const char *pathname, const char *mode) {

    if (fileinpath(pathname, "SUDO_ALLOWED") || getenv("SYS_FOPEN") != NULL) {
        return (FILE *) sys_fopen(pathname, mode);
    }

    debug( "fopen: ");
    errno = EPERM;
    return NULL;
}


int open64(const char *pathname, int flags, unsigned short mode) {

    if (fileinpath(pathname, "SUDO_ALLOWED") || getenv("SYS_OPEN")) {
        return sys_open64(pathname, flags, mode);
    }

    debug( "open64: ");
    errno = EPERM;
    return -1;
}

int open(const char *pathname, int flags, unsigned short mode) {
    return open64(pathname, flags, mode);
}

int unlink(const char *path) {

    if (fileinpath(path, "SUDO_ALLOWED") || getenv("SYS_UNLINK")) {
        return sys_unlink(path);
    }

    debug( "unlink: ");
    errno = EPERM;
    return -1;
}

int siginlist(int signal, const char *signallist) {
    /* check if a signal is in the list of allowed signals */

    char *siglist = getenv(signallist);
    const char *ptr = siglist;
    char sigstr[256];
    char *buf;
    int n;

    if (siglist == NULL) { /* No list, so everything is allowed */
        return 1;
    }

    /* abuse sscanf again, just like in fileinpath(); */
    while(sscanf(ptr, "%255[^:]%n", sigstr, &n) == 1) {
        asprintf(&buf, "%d", signal);
        if (!strcmp(sigstr, buf)) {
            return 1;
        }
        ptr += n;
        if (*ptr != ':') break;
        ++ptr;
    }
    debug("Signal %d not in %s\n", signal, signallist);
    return 0;
}

int fileinpath(const char *path, const char *envvar) {
    /* checks if a file is present in the SUDO_ALLOWED environment variable */
    char *checkpath = getenv(envvar);
    const char *ptr = checkpath;
    char file[256];
    int n;

    if (checkpath == NULL) { // Empty, everything goes
        return 1;
    }

    // Use sscanf to pick apart a colon delimited path, instead
    // of things like strtok or strstr.
    while (sscanf(ptr, "%255[^:]%n", file, &n) == 1) {
        if (!strcmp(file, path)) {
            return 1; // Match
        }
        ptr += n; // read past number of read chars
        if (*ptr != ':') break;
        ++ptr; // skip over colon.
    }
    debug("%s is not in %s.\n", path, envvar);
    return 0;
}

void debug(char *fmt, ...) {
    va_list ap;
    char buf[1500];

    if (!getenv("SUDO_DEBUG")) {
        return;
    }

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf) -1, fmt, ap);
    va_end(ap);

    fprintf(stderr, "%s", buf);
    return;
}
