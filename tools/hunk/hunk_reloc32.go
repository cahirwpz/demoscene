package hunk

import (
	"fmt"
	"io"
	"strings"
)

type Reloc32 struct {
	HunkRef uint32
	Offsets []uint32
}

type HunkReloc32 struct {
	Reloc []Reloc32
}

func readHunkReloc32(r io.Reader) (h HunkReloc32) {
	for {
		count := readLong(r)
		if count == 0 {
			break
		}
		hunkRef := readLong(r)
		offsets := make([]uint32, count)
		for i := 0; i < int(count); i++ {
			offsets[i] = readLong(r)
		}
		h.Reloc = append(h.Reloc, Reloc32{hunkRef, offsets})
	}
	return
}

func (h HunkReloc32) Type() HunkType {
	return HUNK_RELOC32
}

func (h HunkReloc32) String() string {
	var sb strings.Builder
	sb.WriteString("HUNK_RELOC32\n")
	for _, r := range h.Reloc {
		fmt.Fprintf(&sb, "  %d: [%d", r.HunkRef, r.Offsets[0])
		for _, o := range r.Offsets[1:] {
			fmt.Fprintf(&sb, ", %d", o)
		}
		sb.WriteString("]\n")
	}
	return sb.String()
}
