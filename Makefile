CC=gcc
CFLAGS=-g -pedantic -std=gnu17 -Wall -Werror -Wextra

.PHONY: all
all: nyush

nyush: nyush.o builtin.o

nyush.o: nyush.c builtin.h

builtin.o: builtin.c builtin.h

.PHONY: clean
clean:
	rm -f *.o nyush
