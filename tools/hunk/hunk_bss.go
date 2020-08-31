package hunk

import (
	"fmt"
	"io"
)

type HunkBss struct {
	Size uint32
}

func readHunkBss(r io.Reader) (h HunkBss) {
	h.Size = readLong(r) * 4
	return
}

func (h HunkBss) String() string {
	return fmt.Sprintf("HUNK_BSS [%d bytes]\n", h.Size)
}
