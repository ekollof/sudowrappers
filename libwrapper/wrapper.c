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

#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>

typedef int (*sys_unlink)(const char *);

int unlink(const char *path) { 
	// Get original unlink address
	// sys_unlink p = dlsym(RTLD_NEXT, "unlink");	

	fprintf(stderr, "\nWrapped unlink called.\n");

	// Call system "unlink" with our check params
	// return p(path);
	return -1;
}
