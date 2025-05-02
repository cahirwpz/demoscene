CC = gcc
LIBS = sdl2 SDL2_image
CFLAGS = -g -O3 -ffast-math -Wall -Werror $(shell pkg-config --cflags $(LIBS))
LDLIBS = $(shell pkg-config --libs $(LIBS)) -lm

TARGET = donut scene

make: $(TARGET)

donut: donut.c raymarch.h
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)
scene: scene.c raymarch.h
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

clean:
	rm -rf $(TARGET)
