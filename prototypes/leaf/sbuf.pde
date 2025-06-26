import java.util.Comparator;
import java.util.Collections;
import java.util.function.Predicate;

class Span {
  int xs, xe;
  color c;

  Span(int xs, int xe, color c) {
    this.xs = xs;
    this.xe = xe;
    this.c = c;
  }
};

class SpanFirst implements Comparator<Span> {  
  @Override public int compare(Span s1, Span s2) {
    return s1.xs - s2.xs;
  }
}

class SpanDepth implements Comparator<Span> {  
  @Override public int compare(Span s1, Span s2) {
    return s1.c - s2.c;
  }
}

class Segment {
  int ys, ye;
  float xs, xe;
  float dxs, dxe;
  color c;

  Segment(float xs, float xe, float dxs, float dxe, int ys, int ye, color c) {
    this.xs = xs;
    this.xe = xe;
    this.dxs = dxs;
    this.dxe = dxe;
    this.ys = ys;
    this.ye = ye;
    this.c = c;
  }
};

class SegmentFinished implements Predicate<Segment> {
  @Override public boolean test(Segment s) {
    return s.ys >= s.ye;
  }
}

class SegmentBuffer {
  ArrayList<Segment> segments[]; // as many as lines on the screen

  SegmentBuffer() {
    segments = new ArrayList[HEIGHT];

    for (int i = 0; i < HEIGHT; i++) {
      segments[i] = new ArrayList<Segment>();
    }
  }

  void add(Segment s) {
    if (s.ye < 0 || s.ys >= HEIGHT) {
      return;
    }

    // clip against top of the screen
    if (s.ys < 0) {
      s.xs -= s.ys * s.dxs;
      s.xe -= s.ys * s.dxe;
      s.ys = 0;
    }

    // clip against bottom of the screen
    if (s.ye > HEIGHT) {
      s.ye = HEIGHT;
    }

    segments[s.ys].add(s);
  }

  void rasterize() {
    ArrayList<Segment> active = new ArrayList<Segment>();
    BinaryHeap<Span> spans = new BinaryHeap<Span>(new SpanDepth());

    loadPixels();

    for (int y = 0; y < HEIGHT; y++) {
      active.addAll(segments[y]);
      segments[y].clear();

      for (Segment s : active) {
        int xs = floor(s.xs + 0.5);
        int xe = floor(s.xe + 0.5);

        if (xe > 0 && xs < WIDTH) {
          xs = max(xs, 0);
          xe = min(xe, WIDTH);

          spans.add(new Span(xs, xe, s.c));
        }

        s.xs += s.dxs;
        s.xe += s.dxe;
        s.ys += 1;
      }

      active.removeIf(new SegmentFinished());

      spans.heapify();

      while (spans.size() > 0) {
        Span s = spans.pop();

        for (int x = s.xs; x < s.xe; x++) {
          pixels[y * width + x] = s.c;
        }
      }
    }

    updatePixels();
  }
}

SegmentBuffer sbuf = new SegmentBuffer();
