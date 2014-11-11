#!/usr/bin/env python

from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext


setup(
  cmdclass={'build_ext': build_ext},
  ext_modules=[
    Extension("zopfli",
              ["blocksplitter.c", "cache.c", "deflate.c", "gzip_container.c",
               "hash.c", "katajainen.c", "lz77.c", "squeeze.c", "tree.c",
               "util.c", "zlib_container.c", "zopfli_lib.c", "zopfli.pyx"],
              extra_compile_args=["-Wno-unused-value",
                                  "-Wno-unused-function"])]
)
