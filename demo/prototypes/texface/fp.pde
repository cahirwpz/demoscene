int fp16_round(int v) {
  return (v + 32767) & 0xffff0000;
}

int fp16_floor(int v) {
  return v & 0xffff0000;
}

int fp16_ceil(int v) {
  return (v + 65535) & 0xffff0000;
}

int fp16_div(int n, int d) {
  return (int)((long)n * 65536 / d);
}

int fp16_mul(int x, int y) {
  return (int)((long)x * (long)y / 65536);
}

int fp16_int(int v) {
  return (v + 32767) / 65536;
}

int fp16(float v) {
  return (int)(v * 65536);
}