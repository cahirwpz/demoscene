package hunk

import "io"

type HunkBss struct {
	size uint32
}

func (h HunkBss) Name() string {
	return "HUNK_BSS"
}

func readHunkBss(r io.Reader) (h HunkBss) {
	h.size = readLong(r)
	return
}
