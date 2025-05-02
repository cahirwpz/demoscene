CC = clang
CFLAGS = -g -O3 -ffast-math -Wall -Werror $(shell pkg-config --cflags sdl2)
LDLIBS = $(shell pkg-config --libs sdl2) -lm

donut: donut.c

clean:
	rm -rf donut donut.dSYM *~
