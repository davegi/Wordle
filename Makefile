# Using µnit is very simple; just include the header and add the C
# file to your sources.  That said, here is a simple Makefile to build
# the example.

CSTD:=99
OPENMP:=n
ASAN:=n
UBSAN:=n
EXTENSION:=
TEST_ENV:=
CFLAGS:=-g -DMUNIT_TEST_NAME_LEN=45 -D_GNU_SOURCE $(DJG) # -lreadline
AGGRESSIVE_WARNINGS=n

ifeq ($(CC),pgcc)
        CFLAGS+=-c$(CSTD)
else
        CFLAGS+=-std=c$(CSTD)
endif

ifeq ($(OPENMP),y)
        ifeq ($(CC),pgcc)
                CFLAGS+=-mp
        else
                CFLAGS+=-fopenmp
        endif
endif

ifneq ($(SANITIZER),)
        CFLAGS+=-fsanitize=$(SANITIZER)
endif

ifneq ($(CC),pgcc)
        ifeq ($(EXTRA_WARNINGS),y)
                CFLAGS+=-Wall -Wextra -Werror
        endif

        ifeq ($(ASAN),y)
                CFLAGS+=-fsanitize=address
        endif

        ifeq ($(UBSAN),y)
                CFLAGS+=-fsanitize=undefined
        endif
endif

game$(EXTENSION): munit.h game.h game_ut.h munit.c game.c game_ut.c main.c
	$(CC) $(CFLAGS) -o $@ munit.c game_ut.c game.c main.c

valgrind:
	valgrind --leak-check=full --show-leak-kinds=all -s ./game --no-fork --show-stderr # --no-fork

example$(EXTENSION): munit.h munit.c example.c
	$(CC) $(CFLAGS) -o $@ munit.c example.c

test:
	$(TEST_ENV) ./example$(EXTENSION)

clean:
	rm -f example$(EXTENSION)

all: example$(EXTENSION)
