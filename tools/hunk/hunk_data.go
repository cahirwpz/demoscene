package hunk

import "io"

type HunkData struct {
	data []byte
}

func (h HunkData) Name() string {
	return "HUNK_DATA"
}

func readHunkData(r io.Reader) (h HunkData) {
	nlongs := readLong(r)
	h.data = readData(r, nlongs*4)
	return
}
