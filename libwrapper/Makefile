TESTPRG=test
WRAPPER=wrapper.so

all:
	@echo Valid targets: linux freebsd solaris AIX

linux:
	@${MAKE} -f Makefile.linux

freebsd:
	@${MAKE} -f Makefile.freebsd

solaris:
	@${MAKE} -f Makefile.solaris

AIX:
	@${MAKE} -f Makefile.AIX

clean:
	@${RM} -f *.o *.so *.core core *.obj *~ ${TESTPRG}
