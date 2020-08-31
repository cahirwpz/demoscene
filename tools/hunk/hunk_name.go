package hunk

import (
	"fmt"
	"io"
)

type HunkName struct {
	Name string
}

func readHunkName(r io.Reader) (h HunkName) {
	h.Name = readString(r)
	return
}

func (h HunkName) String() string {
	return fmt.Sprintf("HUNK_NAME '%s'\n", h.Name)
}
