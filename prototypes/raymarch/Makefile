CC = clang
CFLAGS = -g -O3 -ffast-math -Wall -Werror $(shell pkg-config --cflags sdl2)
LDLIBS = $(shell pkg-config --libs sdl2) -lm

TARGET = donut scene

make: $(TARGET)

donut: donut.c
scene: scene.c

clean:
	rm -rf $(TARGET)
