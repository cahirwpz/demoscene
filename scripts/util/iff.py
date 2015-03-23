#!/usr/bin/env python

import logging
import struct
import binascii
import StringIO
import collections
from chunk import Chunk


class IffData(StringIO.StringIO):
  def eof(self):
    return self.tell() >= len(self.getvalue())

  def __repr__(self):
    return "<<< binary data of %d bytes >>>" % len(self.getvalue())


class IffChunk(object):
  __slots__ = ('name', 'data')

  def __init__(self, name, data):
    self.name = name
    self.data = data


class IffFile(collections.Sequence):
  ChunkAliasMap = {}
  ChunkBlackList = []

  @classmethod
  def fromFile(cls, filename):
    iff = cls()
    if iff.load(filename):
      return iff

  def __init__(self, form):
    self.form = form
    self.chunks = []

  def load(self, filename):
    self.chunks = []

    with open(filename) as iff:
      chunk = Chunk(iff)

      logging.info('Reading file "%s" as IFF/%s type.' % (filename, self.form))

      if chunk.getname() == 'FORM' and chunk.read(4) == self.form:
        iff.seek(12)

        while True:
          try:
            chunk = Chunk(iff)
          except EOFError:
            break

          name = chunk.getname()
          size = chunk.getsize()
          data = chunk.read()

          if name in self.ChunkBlackList:
            logging.info('Ignoring %s chunk of size %d' % (name, size))
          else:
            logging.debug('Encountered %s chunk of size %d' % (name, size))

            self.chunks.append(self.readChunk(name, data))
      else:
        logging.warn(
          'File %s is not of IFF/%s type.' % (filename, self.form))
        return False

    return True

  def save(self, filename):
    with open(filename, 'w') as iff:
      logging.info('Writing file "%s"' % filename)

      iff.write('FORM' + '\000' * 4 + self.form)

      for chunk in self.chunks:
        data = self.writeChunk(chunk)
        if data:
          iff.write(chunk.name)
          iff.write(struct.pack('>I', len(data)))
          iff.write(data)
          if len(data) % 2 == 1:
            iff.write('\000')

      size = iff.tell() - 8
      iff.seek(4)
      iff.write(struct.pack('>I', size))

  def readChunk(self, name, data):
    orig_name = name

    for alias, names in self.ChunkAliasMap.items():
      if name in names:
        name = alias

    handler = getattr(self, 'read%s' % name, None)
    arg = IffData(data)

    if handler:
      data = handler(arg)
    else:
      data = binascii.hexlify(arg.getvalue())
      logging.warning('No handler for %s chunk.' % orig_name)

    return IffChunk(orig_name, data)

  def writeChunk(self, chunk):
    name = chunk.name

    for alias, names in self.ChunkAliasMap.items():
      if name in names:
        name = alias

    handler = getattr(self, 'write%s' % name, None)
    out = IffData()

    if handler:
      handler(chunk.data, out)
      return out.getvalue()

    logging.warning('No handler for %s chunk.' % chunk.name)
    return None

  def get(self, name, always_list=False):
    chunks = [c for c in self.chunks if c.name == name]

    if not chunks:
      raise ValueError('No chunk named %s.' % name)

    if len(chunks) == 1 and not always_list:
      return chunks[0]
    else:
      return chunks

  def __getitem__(self, name):
    return self.get(name)

  def __iter__(self):
    return iter(self.chunks)

  def __len__(self):
    return len(self.chunks)
