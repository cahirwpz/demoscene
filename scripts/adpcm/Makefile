CC = clang -g
CFLAGS = -O2 -Wall

all: adpcm

adpcm: main.o codec.o
	$(CC) -o $@ $^
codec.o: codec.c codec.h
main.o: main.c codec.h endian.h

clean:
	@rm -vf *.o *~
	@rm -vf adpcm
	@rm -vfr adpcm.dSYM
