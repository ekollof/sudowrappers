SRCS=wrapper.c compat.c
OBJS=wrapper.o compat.o
TSTSRC=test.c
TSTOBJ=test.o
TSTPRG=test
LIB=wrapper.so
WRAPCFLAGS=-fPIC -Wall -shared
WRAPLDFLAGS=-ldl -lc -nostdlib
STRIP=strip


all: test
	$(CC) $(WRAPCFLAGS) $(WRAPLDFLAGS) -o $(LIB) $(SRCS)

release: test
	$(CC) $(WRAPCFLAGS) $(WRAPLDFLAGS) -o $(LIB) $(SRCS)
	$(STRIP) $(LIB)

test: 
	$(CC) -o $(TSTPRG) $(TSTSRC)

clean:
	$(RM) -f *.o *.so $(TSTPRG) $(LIB)
