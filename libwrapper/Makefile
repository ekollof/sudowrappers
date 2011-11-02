SRCS=wrapper.c
OBJS=wrapper.o
LIB=wrapper.so
CFLAGS=-fPIC -Wall -Werror
LDFLAGS=-shared -ldl


all: $(OBJS)
	$(CC) $(LDFLAGS) -o $(LIB)

clean:
	$(RM) -f $(OBJS) $(LIB)
