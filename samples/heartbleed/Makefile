CC=afl-gcc
CFLAGS?=-Wall
LIBS?=
all: heartbleed

clean:
	rm -f heartbleed *.o core core.*

heartbleed: heartbleed.o
	${CC} ${CFLAGS} -o heartbleed heartbleed.o ${LIBS}
