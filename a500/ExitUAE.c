int __nocommandline = 1;
int __initlibraries = 0;

static int (*calltrap)(int i) = (int (*)(int i))0xF0FF60;

int main()
{
  calltrap(13);
  return 0;
}
