#include <dos/dosextens.h>
#include <exec/execbase.h>
#include <proto/exec.h>
#include <setjmp.h>
#include <strings.h>

#include "amigahunk.h"
#include "newstackswap.h"

int __nocommandline = 1;
u_int __oslibversion = 33;

extern char *__commandline;
extern int __commandlen;

struct StackState {
  struct StackSwapArgs args;
  struct StackSwapStruct stack;
  jmp_buf state;
  int retval;
} ss;

void CallIt(int (*fn)(char * asm("a0"), int asm("d0"), void * asm("a6")),
            char *argPtr, int argSize)
{
  if (!setjmp(ss.state)) {
    Log("Calling the program!\n");
    ss.retval = fn(argPtr, argSize, SysBase);
    longjmp(ss.state, 1);
  }
}

int RunIt(BPTR seglist, char *argPtr, int argSize) {
  struct Process *proc = (struct Process *)FindTask(NULL);
  void *stack;

  if ((stack = AllocMem(proc->pr_StackSize, MEMF_ANY))) {
    {
      short n = proc->pr_StackSize / 4;
      int *mem = stack;

      while (n--)
        *mem++ = 0xDEADC0DE;
    }

    ss.stack.stk_Lower = stack;
    ss.stack.stk_Upper = (int)stack + proc->pr_StackSize;
    ss.stack.stk_Pointer = (void *)ss.stack.stk_Upper - sizeof(int);
    ((int *)ss.stack.stk_Pointer)[0] = proc->pr_StackSize;

    ss.args.arg[0] = (int)BADDR(seglist) + sizeof(int);
    ss.args.arg[1] = (int)argPtr;
    ss.args.arg[2] = argSize;
    ss.args.arg[3] = 0xDEADC0DE;
    ss.args.arg[4] = 0xDEADC0DE;
    ss.args.arg[5] = 0xDEADC0DE;
    ss.args.arg[6] = 0xDEADC0DE;
    ss.args.arg[7] = 0xDEADC0DE;

    NewStackSwap(&ss.stack, CallIt, &ss.args);

    FreeMem(stack, proc->pr_StackSize);

    return ss.retval;
  }

  return -1;
}

int main() {
  char *filename = __builtin_alloca(__commandlen + 2);
  char *argPtr;
  int argSize;
  FileT *fh;

  memset(filename, 0, __commandlen + 2);
  memcpy(filename, __commandline, __commandlen);

  argPtr = memchr(filename, ' ', __commandlen);
  if (argPtr) {
    *argPtr++ = '\0';
    argSize = __commandlen - (argPtr - filename);
  } else {
    argPtr = filename + __commandlen - 1;
    *argPtr++ = '\0';
    *argPtr = '\n';
    argSize = 1;
  }

  if ((fh = OpenFile(filename, IOF_BUFFERED))) {
    BPTR seglist;
    Log("Parsing '%s':\n", filename);
    if ((seglist = LoadExecutable(fh))) {
      Log("'%s' returned %d\n", filename, RunIt(seglist, argPtr, argSize));
      FreeSegList(seglist);
    } else {
      Log("'%s' not an Amiga executable file.\n", filename);
    }
    CloseFile(fh);
  } else {
    Log("'%s' no such file.\n", filename);
  }

  Log("Press CTRL+C to finish.\n");
  Wait(SIGBREAKF_CTRL_C);

  return 0;
}
