package hunk

import (
	"fmt"
	"io"
)

type HunkUnit struct {
	Name string
}

func readHunkUnit(r io.Reader) (h HunkUnit) {
	h.Name = readString(r)
	return
}

func (h HunkUnit) String() string {
	return fmt.Sprintf("HUNK_UNIT '%s'\n", h.Name)
}
