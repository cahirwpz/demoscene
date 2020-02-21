package iff

import (
	"bytes"
	"encoding/binary"
	"encoding/hex"
	"errors"
	"fmt"
	"io"
)

var ParsedChunks map[string]func() Chunk
var IgnoredChunks map[string]bool

func init() {
	ParsedChunks = make(map[string]func() Chunk)
	IgnoredChunks = make(map[string]bool)

	ParsedChunks["ANNO"] = makeANNO
}

type Reader interface {
	io.Reader
	io.Seeker
	Size() int64
	Skip(int64)
	ReadU8() uint8
	ReadI8() int8
	ReadU16() uint16
	ReadI16() int16
	ReadI32() int32
}

type Chunk interface {
	fmt.Stringer
	Name() string
	Read(r Reader)
}

type File interface {
	Name() string
	Chunks() []Chunk
}

// IFF chunk reader
type iffReader struct {
	*bytes.Reader
}

func (r *iffReader) ReadU8() (val uint8) {
	if err := binary.Read(r, binary.BigEndian, &val); err != nil {
		panic(err)
	}
	return val
}

func (r *iffReader) ReadI8() (val int8) {
	if err := binary.Read(r, binary.BigEndian, &val); err != nil {
		panic(err)
	}
	return val
}

func (r *iffReader) ReadU16() (val uint16) {
	if err := binary.Read(r, binary.BigEndian, &val); err != nil {
		panic(err)
	}
	return val
}

func (r *iffReader) ReadI16() (val int16) {
	if err := binary.Read(r, binary.BigEndian, &val); err != nil {
		panic(err)
	}
	return val
}

func (r *iffReader) ReadU32() (val uint32) {
	if err := binary.Read(r, binary.BigEndian, &val); err != nil {
		panic(err)
	}
	return val
}

func (r *iffReader) ReadI32() (val int32) {
	if err := binary.Read(r, binary.BigEndian, &val); err != nil {
		panic(err)
	}
	return val
}

func (r *iffReader) Skip(off int64) {
	r.Seek(off, io.SeekCurrent)
}

// IFF chunk
type anyChunk struct {
	name string
	data []byte
}

func (c *anyChunk) Name() string {
	return c.name
}

func (c *anyChunk) Read(r Reader) {
	c.data = make([]byte, r.Size())
	r.Read(c.data)
}

func (c *anyChunk) String() string {
	if len(c.data) > 40 {
		return fmt.Sprintf("{%s...} (%d bytes)", hex.EncodeToString(c.data[:40]),
			len(c.data))
	}
	return fmt.Sprintf("{%s}", hex.EncodeToString(c.data))
}

// IFF file
type iffFile struct {
	name   string
	chunks []Chunk
}

func (f iffFile) Name() string {
	return f.name
}

func (f iffFile) Chunks() []Chunk {
	return f.chunks
}

// helper functions for reading IFF file
func chunkSize(r io.Reader) int {
	var val int32
	if err := binary.Read(r, binary.BigEndian, &val); err != nil {
		panic(err)
	}
	return int(val)
}

func chunkName(r io.Reader) string {
	buf := make([]byte, 4)
	if _, err := io.ReadFull(r, buf); err != nil {
		panic(err)
	}
	return string(buf)
}

func chunkData(r io.Reader, size int) Reader {
	buf := make([]byte, size)
	if _, err := io.ReadFull(r, buf); err != nil {
		panic(err)
	}
	return &iffReader{bytes.NewReader(buf)}
}

func ReadIff(r io.Reader) (iff File, err error) {
	defer func() {
		p := recover()
		if p != nil {
			var ok bool
			err, ok = p.(error)
			if !ok {
				panic(p)
			}
		}
	}()

	form := chunkName(r)
	if form != "FORM" {
		return nil, errors.New("not an IFF file")
	}

	var chunks []Chunk

	size := chunkSize(r)
	name := chunkName(r)

	for size > 4 {
		var c Chunk
		cName := chunkName(r)
		cSize := chunkSize(r)
		cData := chunkData(r, cSize)
		if create, ok := ParsedChunks[cName]; ok {
			c = create()
		} else {
			c = &anyChunk{name: cName}
		}
		c.Read(cData)
		if _, ok := IgnoredChunks[cName]; !ok {
			chunks = append(chunks, c)
		}
		size -= cSize + 8
	}

	return &iffFile{name, chunks}, nil
}
