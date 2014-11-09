#!/usr/bin/env python

from ctypes import cdll, create_string_buffer, memmove
from ctypes import byref, POINTER, CFUNCTYPE
from ctypes import c_int, c_uint, c_char_p
from ctypes.util import find_library


class LZOError(Exception):
  def __init__(self, value):
    self.value = value

  def __str__(self):
    return repr(self.value)


class LZO(object):
  def __init__(self):
    lzo = cdll.LoadLibrary(find_library("lzo"))

    prototype = CFUNCTYPE(c_int, c_char_p, c_uint, c_char_p, POINTER(c_uint),
                          c_char_p)
    paramflags = ((1, "src", None), (1, "src_len", 0),
                  (1, "dst", None), (1, "dst_len", 0),
                  (1, "wrkmem", None))

    self.lzo1x_999_compress = prototype(
      ("lzo1x_999_compress", lzo), paramflags)
    self.lzo1x_decompress = prototype(
      ("lzo1x_decompress", lzo), paramflags)
    self.lzo1x_decompress_safe = prototype(
      ("lzo1x_decompress_safe", lzo), paramflags)
    self.wrkmem = create_string_buffer(14 * 16384 * 4)

  def compress(self, src):
    dst_len = c_uint(len(src) + len(src) / 16 + 64 + 3)
    dst = create_string_buffer(dst_len.value)

    err = self.lzo1x_999_compress(
      src, len(src), dst, byref(dst_len), self.wrkmem)
    if err != 0:
      raise LZOError(err)

    out = create_string_buffer(dst_len.value)
    memmove(out, dst, dst_len.value)

    return out.raw

  def decompress(self, src, dst_len):
    out = create_string_buffer(dst_len)
    out_len = c_uint(dst_len)

    err = self.lzo1x_decompress_safe(
      src, len(src), out, byref(out_len), None)
    if err != 0:
      raise LZOError(err)

    return out
