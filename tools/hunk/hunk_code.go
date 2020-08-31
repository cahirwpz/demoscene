package hunk

import "io"

type HunkCode struct {
	code []byte
}

func (h HunkCode) Name() string {
	return "HUNK_CODE"
}

func readHunkCode(r io.Reader) (h HunkCode) {
	nlongs := readLong(r)
	h.code = readData(r, nlongs*4)
	return
}
