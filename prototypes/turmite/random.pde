import java.util.Random;

Random random = new Random();

int randomInt(int lo, int hi) {
  assert lo < hi;
  return random.nextInt(hi - lo + 1) + lo;
}
