CC = clang
CFLAGS = -g -O3 -ffast-math -Wall -Werror $(shell pkg-config --cflags sdl2)
LDLIBS = $(shell pkg-config --libs sdl2) -lm

raymarch: raymarch.c

clean:
	rm -rf raymarch raymarch.dSYM *~
