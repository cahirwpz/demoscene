package hunk

import (
	"fmt"
	"io"
	"strings"
)

type HunkHeader struct {
	Residents  []string
	Hunks      uint32
	First      uint32
	Last       uint32
	Specifiers []uint32
}

func readHunkHeader(r io.Reader) (h HunkHeader) {
	h.Residents = readArrayOfString(r)
	h.Hunks = readLong(r)
	h.First = readLong(r)
	h.Last = readLong(r)
	n := h.Last - h.First + 1
	h.Specifiers = make([]uint32, n)
	for i := 0; i < int(n); i++ {
		h.Specifiers[i] = readLong(r)
	}
	return
}

func (h HunkHeader) Type() uint32 {
	return HUNK_HEADER
}

func (h HunkHeader) String() string {
	var sb strings.Builder
	sb.WriteString("HUNK_HEADER\n")
	fmt.Fprintf(&sb, "  Residents: %s\n", h.Residents)
	for i, v := range h.Specifiers {
		fmt.Fprintf(&sb, "  Hunk %d: %6d bytes", i+int(h.First), v<<2)
		if v&HUNKF_CHIP != 0 {
			sb.WriteString(" [MEMF_CHIP]\n")
		} else if v&HUNKF_FAST != 0 {
			sb.WriteString(" [MEMF_FAST]\n")
		} else {
			sb.WriteString(" [MEMF_PUBLIC]\n")
		}
	}
	return sb.String()
}
