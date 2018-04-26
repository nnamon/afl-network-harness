CC=afl-gcc
CFLAGS?=-Wall
LIBS?=
all: authed

clean:
	rm -f goribble *.o core core.*

authed: authed.o
	${CC} ${CFLAGS} -o authed authed.o ${LIBS}
