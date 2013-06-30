CC = clang -g
CFLAGS = -Wall -O2

all: test-json

test-json: test-json.o json.o json-print.o query.o

json.o: json.c json.h
json-print.o: json-print.c json.h
query.o: query.c json.h
test-json.o: test-json.c json.h

clean:
	@rm -vf *.o *.a *~
	@rm -vf test-json
	@rm -vrf test-json.dSYM
