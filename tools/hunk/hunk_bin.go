package hunk

import (
	"bytes"
	"encoding/hex"
	"fmt"
	"io"
)

// Compression Type: This is private, non-standard extension.
// Valid only for loadable files, but does not clash with
// linkable hunk type.
type CompType uint32

const (
	COMP_NONE CompType = 0 << 28
	COMP_LZSA CompType = 1 << 28
	COMP_ZX0  CompType = 3 << 28
	COMP_MASK CompType = 3 << 28

	BHF_FLAG_MASK = HUNKF_FLAG_MASK
	BHF_COMP_MASK = uint32(COMP_MASK)
	BHF_SIZE_MASK = ^(BHF_COMP_MASK | BHF_FLAG_MASK)
)

func (comp CompType) String() string {
	switch comp & COMP_MASK {
	case COMP_NONE:
		return "NONE"
	case COMP_LZSA:
		return "LZSA"
	case COMP_ZX0:
		return "ZX0"
	default:
		return "???"
	}
}

func binHunkSpec(x uint32) (HunkFlag, CompType, uint32) {
	return HunkFlag(x & BHF_FLAG_MASK), CompType(x & BHF_COMP_MASK), (x & BHF_SIZE_MASK) * 4
}

type HunkBin struct {
	htype HunkType
	Flags HunkFlag
	Comp  CompType
	Data  *bytes.Buffer
}

func readHunkBin(r io.Reader, htype HunkType) *HunkBin {
	flags, comp, size := binHunkSpec(readLong(r))
	return &HunkBin{htype, flags, comp, bytes.NewBuffer(readData(r, size))}
}

func (h HunkBin) Type() HunkType {
	return h.htype
}

func (h HunkBin) Write(w io.Writer) {
	writeLong(w, uint32(h.htype))
	writeLong(w, uint32(h.Flags)|uint32(h.Comp)|bytesSize(h.Data.Bytes()))
	writeData(w, h.Data.Bytes())
}

func (h HunkBin) String() string {
	comp := ""
	if h.Comp != 0 {
		comp = fmt.Sprintf("compressed: %s, ", h.Comp.String())
	}
	return fmt.Sprintf("%s [%s, %s%d bytes]\n%s", h.Type().String(),
		h.Flags.String(), comp, h.Data.Len(), hex.Dump(h.Data.Bytes()))
}
