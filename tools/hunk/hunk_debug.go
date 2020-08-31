package hunk

import "io"

type HunkDebug struct {
	Data []byte
}

func readHunkDebug(r io.Reader) (h HunkDebug) {
	nlongs := readLong(r)
	h.Data = readData(r, nlongs*4)
	return
}

func (h HunkDebug) String() string {
	return "HUNK_DEBUG\n"
}
