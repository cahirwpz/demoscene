package hunk

import (
	"encoding/binary"
	"io"
	"os"
)

func writeLong(w io.Writer, x uint32) {
	if err := binary.Write(w, binary.BigEndian, x); err != nil {
		panic("failed to write long")
	}
}

func writeArrayOfLong(w io.Writer, xs []uint32) {
	writeLong(w, uint32(len(xs)))
	for _, v := range xs {
		writeLong(w, v)
	}
}

func bytesSize(xs []byte) uint32 {
	return uint32((len(xs) + 3) / 4)
}

func stringSize(s string) uint32 {
	return uint32((len(s) + 3) / 4)
}

func bytesPadding(xs []byte) int {
	k := len(xs) % 4
	if k > 0 {
		return 4 - k
	}
	return 0
}

func writeData(w io.Writer, xs []byte) {
	if err := binary.Write(w, binary.BigEndian, xs); err != nil {
		panic("failed to write data")
	}
	if p := bytesPadding(xs); p > 0 {
		for i := 0; i < p; i++ {
			if err := binary.Write(w, binary.BigEndian, byte(0)); err != nil {
				panic("failed to write padding")
			}
		}
	}
}

func writeDataWithSize(w io.Writer, xs []byte) {
	writeLong(w, bytesSize(xs))
	writeData(w, xs)
}

func writeString(w io.Writer, s string) {
	writeData(w, []byte(s))
}

func writeStringWithSize(w io.Writer, s string) {
	writeLong(w, stringSize(s))
	writeString(w, s)
}

func writeArrayOfString(w io.Writer, ss []string) {
	for _, s := range ss {
		writeString(w, s)
	}
	writeLong(w, 0)
}

func WriteFile(path string, hunks []Hunk, perm os.FileMode) (err error) {
	file, err := os.OpenFile(path, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, perm)
	if err != nil {
		return
	}

	defer file.Close()

	for _, hunk := range hunks {
		hunk.Write(file)
	}

	return nil
}
