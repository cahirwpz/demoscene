package hunk

import (
	"fmt"
	"io"
)

type HunkBss struct {
	Size uint32
}

func readHunkBss(r io.Reader) HunkBss {
	return HunkBss{readLong(r) * 4}
}

func (h HunkBss) Type() uint32 {
	return HUNK_BSS
}

func (h HunkBss) String() string {
	return fmt.Sprintf("%s [%d bytes]\n", HunkNameMap[h.Type()], h.Size)
}
