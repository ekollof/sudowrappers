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
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
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
 * to ensure correct behavior. 
 *
 * The following calls will be wrapped:
 * unlink, fopen, open, stat, exec
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

/* prototypes */
int fileinpath(const char *, const char *);
void debug(char *, ...);


int (*sys_unlink)(const char *);
int (*sys_open)(const char *, int, unsigned short);
int (*sys_open64)(const char *, int, unsigned short);
FILE *(*sys_fopen)(const char *pathname, const char *mode);
int (*sys_rename)(const char *, const char *);
sighandler_t (*sys_signal)(int, sighandler_t);
int (*sys_unlinkat)(int dirfd, const char *pathname, int flags);

void _init(void) {
	sys_unlink = dlsym(RTLD_NEXT, "unlink");	
	sys_open = dlsym(RTLD_NEXT, "open");
	sys_open64 = dlsym(RTLD_NEXT, "open64");
	sys_fopen = dlsym(RTLD_NEXT, "fopen");
	sys_rename = dlsym(RTLD_NEXT, "rename");
	sys_signal = dlsym(RTLD_NEXT, "signal");
	sys_unlinkat = dlsym(RTLD_NEXT, "unlinkat");
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

sighandler_t signal(int signum, sighandler_t handler) {
	
	return sys_signal(signum, handler);
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
