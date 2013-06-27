from libc.math cimport sqrt
from heapq import heappop, heappush
cimport cython

cdef class Color:
  cdef readonly int r, g, b

  def __cinit__(self, int r, int g, int b):
    self.r = r
    self.g = g
    self.b = b

  @cython.cdivision(True)
  def __div__(Color self, float s):
    cdef int r, g, b
    r = int(self.r / s)
    g = int(self.g / s)
    b = int(self.b / s)
    return Color(r, g, b)

  def __str__(self):
    return "(%d, %d, %d)" % (self.r, self.g, self.b)

  cpdef int comp(Color self, int i):
    if i == 0:
      return self.r
    elif i == 1:
      return self.g
    elif i == 2:
      return self.b
    else:
      raise IndexError

  cpdef int max_comp(self):
    return max((self.r, 0), (self.g, 1), (self.b, 2))[1]


cdef float LumaCCIR601(Color color):
  return 0.299 * color.r + 0.587 * color.g + 0.114 * color.b


cdef class Box:
  cdef readonly Color average, color
  cdef readonly list data
  cdef readonly int begin, end, count, weight, axis

  def __init__(self, list data, int begin, int end, average=None):
    assert begin < end

    self.data = data
    self.begin = begin
    self.end = end
    self.average = average or self.CalcAverage()
    self.count = self.end - self.begin
    self.color = self.average / self.count

    variance = self.CalcVariance()

    self.axis = variance.max_comp()
    self.weight = variance.comp(self.axis)

  def __repr__(self):
    return 'Box([%d..%d], count: %d, weight: %s)' % (
      self.begin, self.end, self.count, self.weight)

  def CalcAverage(self):
    cdef Color pixel
    cdef int i, r, g, b

    r, g, b = 0, 0, 0

    for i in range(self.begin, self.end):
      pixel = self.data[i]
      r += pixel.r
      g += pixel.g
      b += pixel.b

    return Color(r, g, b)

  cpdef Color CalcVariance(self):
    cdef Color avg, pixel
    cdef int i, r, g, b, dr, dg, db

    r, g, b = 0, 0, 0
    avg = self.color

    for i in range(self.begin, self.end):
      pixel = self.data[i]
      dr, dg, db = pixel.r - avg.r, pixel.g - avg.g, pixel.b - avg.b
      r += dr * dr
      g += dg * dg
      b += db * db

    return Color(r, g, b)

  def Split(self):
    cdef Color tmp, pixel
    cdef int axis, i, j, c, median
    cdef list data
    cdef list values

    data = self.data
    axis = self.axis
    median = self.color.comp(axis)

    # Sometimes average color is not a good median (ie. when a single value
    # dominates).  In such cases we need to take the other value.

    # Sort by counting.
    values = [0 for i in range(256)]
    for i in range(self.begin, self.end):
      pixel = data[i]
      j = pixel.comp(axis)
      values[j] += 1
    values = [i for i in values if i > 0]

    if median == values[-1]:
      median = values[-2]
    if median == values[0]:
      median = values[1]

    i = self.begin
    j = self.end - 1

    while i < j:
      while data[i].comp(axis) < median and i < j:
        i += 1
      while data[j].comp(axis) >= median and i < j:
        j -= 1

      tmp = data[i]
      data[i] = data[j]
      data[j] = tmp

    boxL = Box(self.data, self.begin, i)
    # averageR = self.average - boxL.average
    boxR = Box(self.data, i, self.end)

    return (axis, median, boxL, boxR)


cdef class KDNode:
  cdef readonly int axis, median
  cdef public int number
  cdef readonly Box box
  cdef readonly KDNode left, right

  def __init__(self, Box box):
    self.box = box
    self.left = None
    self.right = None
    self.number = -1
    self.axis = 0
    self.median = 0

  def __cmp__(self, KDNode other):
    cdef int d = other.box.weight - self.box.weight

    if d < 0:
      return -1
    elif d > 0:
      return 1
    else:
      return 0

  def __repr__(self):
    return "KDTree{%r}" % self.box

  def Split(self):
    self.axis, self.median, left, right = self.box.Split()
    self.left = KDNode(left)
    self.right = KDNode(right)
    self.box = None

  cpdef tuple Search(KDNode self, Color color):
    cdef Color avg
    cdef int dist, r, g, b
    cdef float error, alt_error
    cdef KDNode node, alt_child

    if self.box:
      avg = self.box.color
      r, g, b = color.r - avg.r, color.g - avg.g, color.b - avg.b
      return (self, Color(r, g, b), sqrt(float(r * r + g * g + b * b)))

    dist = color.comp(self.axis) - self.median

    if dist < 0:
      child, alt_child = self.left, self.right
    else:
      child, alt_child = self.right, self.left

    res = child.Search(color)
    error = res[2]

    if error > abs(dist):
      alt_res = alt_child.Search(color)
      alt_error = alt_res[2]

      if alt_error < error:
        return alt_res

    return res


def SplitKDTree(KDNode kdtree, unsigned int leavesNum):
  cdef list leaves
  cdef KDNode leaf

  leaves = [kdtree]

  while len(leaves) < leavesNum:
    leaf = heappop(leaves)
    leaf.Split()
    heappush(leaves, leaf.left)
    heappush(leaves, leaf.right)

  return sorted(leaves, key=lambda l: LumaCCIR601(l.box.color))


cpdef tuple AddErrorAndClamp(tuple pixel, Color error, int coeff):
  cdef int r, g, b

  r, g, b = pixel

  r += (error.r * coeff + 7) / 16
  g += (error.g * coeff + 7) / 16
  b += (error.b * coeff + 7) / 16

  if r < 0:
    r = 0
  if r > 255:
    r = 255
  if g < 0:
    g = 0
  if g > 255:
    g = 255
  if b < 0:
    b = 0
  if b > 255:
    b = 255

  return (r, g, b)


cpdef FloydSteinberg(pixels, int x, int y, int width, int height, Color error):
  if x < width - 1:
    pixels[x + 1, y] = AddErrorAndClamp(pixels[x + 1, y], error, 7)

  if y < height - 1:
    if x > 0:
      pixels[x - 1, y + 1] = AddErrorAndClamp(pixels[x - 1, y + 1], error, 3)

    pixels[x, y + 1] = AddErrorAndClamp(pixels[x, y + 1], error, 5)

    if x < width - 1:
      pixels[x + 1, y + 1] = AddErrorAndClamp(pixels[x + 1, y + 1], error, 1)
