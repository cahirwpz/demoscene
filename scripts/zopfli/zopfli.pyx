from cpython.mem cimport PyMem_Malloc, PyMem_Free
from cpython cimport PyObject_Size
from libc.stdio cimport printf
from libc.stdlib cimport malloc, free

cdef extern from "zopfli.h":
  ctypedef struct ZopfliOptions:
    int verbose;
    int verbose_more;
    int numiterations;
    int blocksplitting;
    int blocksplittinglast;
    int blocksplittingmax;

  ctypedef enum ZopfliFormat:
    ZOPFLI_FORMAT_GZIP = 0
    ZOPFLI_FORMAT_ZLIB = 1
    ZOPFLI_FORMAT_DEFLATE = 2

  void ZopfliInitOptions(ZopfliOptions *options);

  void ZopfliCompress(const ZopfliOptions *options,
                      ZopfliFormat output_type,
                      const unsigned char *src, size_t srcsize,
                      unsigned char **out, size_t *outsize);

cdef class Options:
  cdef ZopfliOptions *data

  def __cinit__(self):
    self.data = <ZopfliOptions *> PyMem_Malloc(sizeof(ZopfliOptions))
    if not self.data:
      raise MemoryError()
    ZopfliInitOptions(self.data)

  property verbose:
    def __get__(self):
      return self.data.verbose
    def __set__(self, value):
      self.data.verbose = value

  property verbose_more:
    def __get__(self):
      return self.data.verbose_more
    def __set__(self, value):
      self.data.verbose_more = value

  property numiterations:
    def __get__(self):
      return self.data.numiterations
    def __set__(self, value):
      self.data.numiterations = value

  property blocksplitting:
    def __get__(self):
      return self.data.blocksplitting
    def __set__(self, value):
      self.data.blocksplitting = value

  property blocksplittinglast:
    def __get__(self):
      return self.data.blocksplittinglast
    def __set__(self, value):
      self.data.blocksplittinglast = value

  property blocksplittingmax:
    def __get__(self):
      return self.data.blocksplittingmax
    def __set__(self, value):
      self.data.blocksplittingmax = value

  def __dealloc__(self):
    PyMem_Free(self.data)

cpdef enum Format:
  GZIP = ZOPFLI_FORMAT_GZIP
  ZLIB = ZOPFLI_FORMAT_ZLIB
  DEFLATE = ZOPFLI_FORMAT_DEFLATE

def Compress(Options options, ZopfliFormat fmt, bytes src, size_t outsize):
  cdef size_t srcsize = PyObject_Size(src)
  cdef unsigned char *out = <unsigned char *>malloc(outsize)
  cdef bytes py_string

  outsize = 0

  ZopfliCompress(options.data, fmt,
                 <const unsigned char *>src, srcsize, &out, &outsize)

  try:
    py_string = out[:outsize]
  finally:
    free(out)

  return py_string
