class BinaryHeap<E> extends ArrayList<E> {
  Comparator<E> cmp;

  BinaryHeap(Comparator<E> cmp) {
    super();
    this.cmp = cmp;
  }

  int parent(int i) { 
    return (i - 1) / 2;
  }

  int left(int i) { 
    return 2 * i + 1;
  }

  int right(int i) {
    return 2 * i + 2;
  }

  void swap(int i, int j) {
    E tmp = get(i);
    set(i, get(j));
    set(j, tmp);
  }

  boolean less(int i, int j) {
    return cmp.compare(get(i), get(j)) < 0;
  }

  E pop() {
    int last = size() - 1;
    swap(0, last);
    E e = remove(last);
    siftDown(0);
    return e;
  }
  
  E peek() {
    return get(0);
  }
  
  void push(E e) {
    add(e);
    siftUp(size() - 1);
  }

  void heapify() {
    for (int i = size() / 2 - 1; i >= 0; i--) {
      siftDown(i);
    }
  }

  void siftUp(int i) {
    while (i > 0 && less(i, parent(i))) {
      swap(i, parent(i));
      i = parent(i);
    }
  }

  void siftDown(int i) {
    int n = size();

    while (i < n) {
      int l = left(i);
      int r = right(i);
      int j = i;

      if (l < n && less(l, j)) {
        j = l;
      }

      if (r < n && less(r, j)) {
        j = r;
      }

      if (i == j) {
        break;
      }

      swap(i, j);
      i = j;
    }
  }
};
