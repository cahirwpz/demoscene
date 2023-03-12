class SortedQueue<E extends Comparable> {
  E[] elems;
  int tail, head;

  SortedQueue(int nelems) {
    elems = (E[])new Comparable[nelems];
    head = 0;
    tail = 0;
  }
  
  int index(int i) {
    i = i % elems.length;
    if (i < 0) {
      i += elems.length;
    }
    return i;
  }
  
  boolean full() {
    return index(head + 1) == index(tail);
  }
  
  boolean empty() {
    return index(head) == index(tail);
  }
  
  void add(E elem) {
    assert(!full());

    // keep them sorted by descending diameter
    
    int prev = index(head - 1);
    int curr = head;
    
    while (curr != tail && elem.compareTo(elems[prev]) <= 0) {
      elems[curr] = elems[prev];      
      curr = prev;
      prev = index(prev - 1);
    }

    elems[curr] = elem;
    head = index(head + 1);
  }
  
  E get(int i) {
    assert(!empty());
    assert(i < size());
    
    return elems[index(head - 1 - i)];
  }
  
  E last() {
    return get(size() - 1);
  }
  
  void pop() {
    assert(!empty());
    tail = index(tail + 1);
  }
  
  void clear() {
    for (int i = 0; i < elems.length; i++) {
      elems[i] = null;
    }
    head = 0;
    tail = 0;
  }
  
  int size() {
    int size = head - tail;
    if (size < 0) {
      size += elems.length;
    }
    return size;
  }
}
