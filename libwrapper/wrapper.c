/* 
 * This is libwrapper, part of the sudowrapper suite by 
 * Emiel Kollof <emiel.kollof@nl.ibm.com>.
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

extern int errno;

/* prototypes */
int fileinpath(const char *, const char *);


int (*sys_unlink)(const char *);
int (*sys_open)(const char *, int, unsigned short);
int (*sys_open64)(const char *, int, unsigned short);
FILE *(*sys_fopen)(const char *pathname, const char *mode);

void _init(void) {
	sys_unlink = dlsym(RTLD_NEXT, "unlink");	
	sys_open = dlsym(RTLD_NEXT, "open");
	sys_open64 = dlsym(RTLD_NEXT, "open64");
	sys_fopen = dlsym(RTLD_NEXT, "fopen");
}

FILE *fopen(const char *pathname, const char *mode) {

	if (fileinpath(pathname, "SUDO_ALLOWED")) {
		return (FILE *) sys_fopen(pathname, mode);
	}

	errno = EPERM;
	return NULL;
}


int open(const char *pathname, int flags, unsigned short mode) {

	if (fileinpath(pathname, "SUDO_ALLOWED")) {	
		return sys_open(pathname, flags, mode);
	} 
		
	errno = EPERM;
	return -1;
}

int open64(const char *pathname, int flags, unsigned short mode) {
	return open(pathname, flags, mode);
}

int unlink(const char *path) { 

	printf("\nunlink wrapped\n");

	if (fileinpath(path, "SUDO_ALLOWED")) { 
		return sys_unlink(path);
	} else { 
		errno = EPERM;
		return -1;
	}
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
	return 0;
}
