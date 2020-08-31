package hunk

import (
	"encoding/hex"
	"fmt"
	"io"
)

type HunkCode struct {
	Code []byte
}

func readHunkCode(r io.Reader) (h HunkCode) {
	nlongs := readLong(r)
	h.Code = readData(r, nlongs*4)
	return
}

func (h HunkCode) String() string {
	return fmt.Sprintf("HUNK_CODE [%d bytes]\n%s", len(h.Code), hex.Dump(h.Code))
}
