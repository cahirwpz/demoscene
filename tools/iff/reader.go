package iff

import (
	"bytes"
	"encoding/binary"
	"io"
)

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
	ReadU32() uint32
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
