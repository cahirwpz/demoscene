#!/usr/bin/env python2.7

from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext


setup(
  cmdclass = {'build_ext': build_ext},
  ext_modules = [
    Extension("quant",
              ["quant.pyx"],
              extra_compile_args=["-Wno-unused-value","-Wno-unused-function"])]
)
