package hunk

import "io"

type HunkDebug struct {
	data []byte
}

func (h HunkDebug) Name() string {
	return "HUNK_DEBUG"
}

func readHunkDebug(r io.Reader) (h HunkDebug) {
	nlongs := readLong(r)
	h.data = readData(r, nlongs*4)
	return
}
