color[] MakePalette(color[] input) {
  color[] output = new int[256];
  float delta = input.length / 256.0;
  float base = 0.0;
  color c_lo = #000000;
  color c_hi = input[0];
  color c_index = 0;
  for (int i = 0; i < 256; i++) {
    if (base >= 1.0) {
      base -= 1.0;
      c_lo = input[c_index];
      c_hi = input[c_index + 1];
      c_index++;
    }
    output[i] = lerpColor(c_lo, c_hi, base);
    base += delta;
  }
  return output;
}

final color[] sem_nome = { #483176, #6FABE7, #95B773, #F2E18D, #F6C95E };
final color[] tropical_fruit_punch = { #146152, #44803F, #B4CF66, #FFEC5C, #FF5A33 };
final color[] box_model = { #354B8C, #4478A6, #50D4F2, #F2A057, #F27B35 };

color[] palette = MakePalette(box_model);
