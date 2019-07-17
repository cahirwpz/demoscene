static int (*calltrap)(int i) = (int (*)(int i))0xF0FF60;

void ExitUAE(void) {
  calltrap(13);
}
