package hunk

import (
	"encoding/hex"
	"fmt"
	"io"
)

type HunkData struct {
	Data []byte
}

func readHunkData(r io.Reader) (h HunkData) {
	nlongs := readLong(r)
	h.Data = readData(r, nlongs*4)
	return
}

func (h HunkData) String() string {
	return fmt.Sprintf("HUNK_DATA [%d bytes]\n%s", len(h.Data), hex.Dump(h.Data))
}
