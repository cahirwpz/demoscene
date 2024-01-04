package hunk

import (
	"encoding/hex"
	"fmt"
	"io"
)

type HunkBin struct {
	htype HunkType
	Flags HunkFlag
	Bytes []byte
}

func readHunkBin(r io.Reader, htype HunkType) *HunkBin {
	flags, size := hunkSpec(readLong(r))
	return &HunkBin{htype, flags, readData(r, size)}
}

func (h HunkBin) Type() HunkType {
	return h.htype
}

func (h HunkBin) Write(w io.Writer) {
	writeLong(w, uint32(h.htype))
	writeLong(w, uint32(h.Flags)|bytesSize(h.Bytes))
	writeData(w, h.Bytes)
}

func (h HunkBin) String() string {
	return fmt.Sprintf("%s [%s, %d bytes]\n%s", h.Type().String(),
		h.Flags.String(), len(h.Bytes), hex.Dump(h.Bytes))
}
