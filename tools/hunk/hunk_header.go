package hunk

import "io"

type HunkHeader struct {
	residents  []string
	hunks      uint32
	first      uint32
	last       uint32
	specifiers []uint32
}

func (h HunkHeader) Name() string {
	return "HUNK_HEADER"
}

func readHunkHeader(r io.Reader) (h HunkHeader) {
	h.residents = readStringArray(r)
	h.hunks = readLong(r)
	h.first = readLong(r)
	h.last = readLong(r)
	n := h.last - h.first + 1
	h.specifiers = make([]uint32, n)
	for i := 0; i < int(n); i++ {
		h.specifiers[i] = readLong(r)
	}
	return
}
