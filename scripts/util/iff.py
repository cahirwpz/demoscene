#!/usr/bin/env python

from chunk import Chunk
from StringIO import StringIO as OrigStringIO
import logging


class StringIO(OrigStringIO):
  def __len__(self):
    return len(self.getvalue())


class Parser(object):
  ChunkAliasMap = {}

  def __init__(self, kind):
    self._kind = kind
    self._chunks = []

  def loadFile(self, filename):
    with open(filename) as iff:
      chunk = Chunk(iff)

      logging.info('Reading file "%s"' % filename)

      if chunk.getname() == 'FORM' and chunk.read(4) == self._kind:
        iff.seek(12)

        while True:
          try:
            chunk = Chunk(iff)
          except EOFError:
            break

          name = chunk.getname()
          size = chunk.getsize()
          data = chunk.read()

          logging.debug('Encountered %s chunk of size %d' % (name, size))

          self._chunks.append(self._parseChunk(name, data))
      else:
        logging.error(
          'File %s is not of IFF/%s type.' % (filename, self._kind))
        return False

    return True

  def _parseChunk(self, name, data):
    orig_name = name

    for alias, names in self.ChunkAliasMap.items():
      if name in names:
        name = alias

    handler = getattr(self, 'handle%s' % name, None)
    arg = data

    if not handler:
      handler = getattr(self, 'read%s' % name, None)
      arg = StringIO(data)

    if handler:
      data = handler(arg)
    else:
      logging.warning('No handler for %s chunk.' % orig_name)

    return (orig_name, data)

  def _getChunk(self, name, always_list=False):
    chunks = [c for n, c in self._chunks if n == name]

    if not chunks:
      raise ValueError('No chunk named %s.' % name)

    if len(chunks) == 1 and not always_list:
      return chunks[0]
    else:
      return chunks
