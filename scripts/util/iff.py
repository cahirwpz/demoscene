#!/usr/bin/env python

import logging
import struct
import binascii

from chunk import Chunk as IffChunk
from collections import Sequence
from StringIO import StringIO as OrigStringIO


class StringIO(OrigStringIO):
  def __len__(self):
    return len(self.getvalue())

  def __repr__(self):
    return 'Data stream of %d bytes' % len(self)


class Chunk(object):
  __slots__ = ('name', 'data')

  def __init__(self, name, data):
    self.name = name
    self.data = data

  def __repr__(self):
    return "%s: %r" % (self.name, self.data)


class File(Sequence):
  ChunkAliasMap = {}
  ChunkBlackList = []

  def __init__(self, kind):
    self._kind = kind
    self._chunks = []

  def load(self, filename):
    self._chunks = []

    with open(filename) as iff:
      chunk = IffChunk(iff)

      logging.info('Reading file "%s"' % filename)

      if chunk.getname() == 'FORM' and chunk.read(4) == self._kind:
        iff.seek(12)

        while True:
          try:
            chunk = IffChunk(iff)
          except EOFError:
            break

          name = chunk.getname()
          size = chunk.getsize()
          data = chunk.read()

          if name in self.ChunkBlackList:
            logging.info('Ignoring %s chunk of size %d' % (name, size))
          else:
            logging.debug('Encountered %s chunk of size %d' % (name, size))

            self._chunks.append(self._readChunk(name, data))
      else:
        logging.error(
          'File %s is not of IFF/%s type.' % (filename, self._kind))
        return False

    return True

  def save(self, filename):
    with open(filename, 'w') as iff:
      logging.info('Writing file "%s"' % filename)

      iff.write('FORM' + '\000' * 4 + self._kind)

      for chunk in self._chunks:
        data = self._writeChunk(chunk)
        if data:
          iff.write(chunk.name)
          iff.write(struct.pack('>I', len(data)))
          iff.write(data)
          if len(data) % 2 == 1:
            iff.write('\000')

      size = iff.tell() - 8
      iff.seek(4)
      iff.write(struct.pack('>I', size))

  def _readChunk(self, name, data):
    orig_name = name

    for alias, names in self.ChunkAliasMap.items():
      if name in names:
        name = alias

    handler = getattr(self, 'read%s' % name, None)
    arg = StringIO(data)

    if handler:
      data = handler(arg)
    else:
      data = binascii.hexlify(arg.getvalue())
      logging.warning('No handler for %s chunk.' % orig_name)

    return Chunk(orig_name, data)

  def _writeChunk(self, chunk):
    name = chunk.name

    for alias, names in self.ChunkAliasMap.items():
      if name in names:
        name = alias

    handler = getattr(self, 'write%s' % name, None)
    out = StringIO()

    if handler:
      handler(chunk.data, out)
      return out.getvalue()

    logging.warning('No handler for %s chunk.' % chunk.name)
    return None

  def get(self, name, always_list=False):
    chunks = [c for c in self._chunks if c.name == name]

    if not chunks:
      raise ValueError('No chunk named %s.' % name)

    if len(chunks) == 1 and not always_list:
      return chunks[0]
    else:
      return chunks

  def __getitem__(self, name):
    return self.get(name)

  def __iter__(self):
    return iter(self._chunks)

  def __len__(self):
    return len(self._chunks)
