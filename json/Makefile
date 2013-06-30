TOPDIR = $(realpath $(CURDIR)/..)

OBJS = lexer.o parser.o json.o node-print.o token-print.o query.o

all:: libjson.a test-json

test-json: LDFLAGS += -L$(TOPDIR)/system -L$(TOPDIR)/std -L.
test-json: LDLIBS += -lsystem -lstd -lm 
test-json: test-json.o libjson.a

libjson.a: $(OBJS)

clean::
	@rm -vf test-json 

include $(TOPDIR)/Makefile.common
