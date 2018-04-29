# Use -static-libasan if you want to compile with ASAN
CC=afl-gcc
CFLAGS?=-Wall
LIBS?=
all: authed

clean:
	rm -f authed *.o core core.*

authed: authed.o
	${CC} ${CFLAGS} -o authed authed.o ${LIBS}
