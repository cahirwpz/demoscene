#include "std/exception.h"
#include <stdio.h>

void baz(int argc) {
  puts("=> baz");

  TRY {
    if (argc > 2) {
      RAISE;
    }
  } CATCH {
    puts("baz catch!");
  }

  puts("<= baz");
}

void foo(int argc) {
  puts("=> foo");

  if (argc > 1) {
    baz(argc);
    RAISE;
  }

  puts("<= foo");
}
 
int main(int argc, char **argv) {
  puts("=> main");

  TRY {
    foo(argc);
  } 
  CATCH {
    puts("main catch!");
  }

  puts("<= main");

  return 0;
}
