package hunk

import (
	"fmt"
	"io"
	"sort"
	"strings"
)

type Reloc32 struct {
	HunkRef uint32
	Offsets []uint32
}

type HunkReloc32 struct {
	Relocs []Reloc32
}

func readHunkReloc32(r io.Reader) *HunkReloc32 {
	var relocs []Reloc32
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
		relocs = append(relocs, Reloc32{hunkRef, offsets})
	}
	return &HunkReloc32{relocs}
}

func (h *HunkReloc32) Sort() {
	sort.Slice(h.Relocs, func(i, j int) bool {
		return h.Relocs[i].HunkRef < h.Relocs[j].HunkRef
	})
}

func (h HunkReloc32) Type() HunkType {
	return HUNK_RELOC32
}

func (h HunkReloc32) Write(w io.Writer) {
	writeLong(w, uint32(HUNK_RELOC32))
	for _, rs := range h.Relocs {
		writeLong(w, uint32(len(rs.Offsets)))
		writeLong(w, rs.HunkRef)
		for _, off := range rs.Offsets {
			writeLong(w, off)
		}
	}
	writeLong(w, 0)
}

func (h HunkReloc32) String() string {
	var sb strings.Builder
	sb.WriteString("HUNK_RELOC32\n")
	for _, r := range h.Relocs {
		fmt.Fprintf(&sb, "  %d: [%d", r.HunkRef, r.Offsets[0])
		for _, o := range r.Offsets[1:] {
			fmt.Fprintf(&sb, ", %d", o)
		}
		sb.WriteString("]\n")
	}
	return sb.String()
}
