package iff

import "io"

type Writer interface {
	io.Writer
	io.Seeker
	Skip(int64)
	WriteU8(uint8)
	WriteI8(int8)
	WriteU16(uint16)
	WriteI16(int16)
	WriteI32(int32)
	WriteU32(uint32)
}
