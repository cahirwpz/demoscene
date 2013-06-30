CC = clang -g
CFLAGS = -Wall -O2

all: test-json

test-json: test-json.o json.o node-print.o query.o token-print.o lexer.o parser.o

json.o: json.c json.h
lexer.o: lexer.c lexer.h
parser.o: parser.c parser.h
token-print.o: token-print.c lexer.h
node-print.o: node-print.c json.h
query.o: query.c json.h
test-json.o: test-json.c json.h

clean:
	@rm -vf *.o *.a *~
	@rm -vf test-json
	@rm -vrf test-json.dSYM
