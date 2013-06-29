CC = clang -g
CFLAGS = -Wall -O2

all: test-json

test-json: json.o test-json.o

json.o: json.c json.h
test-json.o: test-json.c json.h

clean:
	@rm -v *.o
	@rm -vf test-json *~
	@rm -vrf test-json.dSYM
