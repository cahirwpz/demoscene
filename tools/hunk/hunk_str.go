package hunk

import (
	"fmt"
	"io"
)

type HunkStr struct {
	htype HunkType
	Name  string
}

func readHunkName(r io.Reader) HunkStr {
	return HunkStr{HUNK_NAME, readString(r)}
}

func readHunkUnit(r io.Reader) HunkStr {
	return HunkStr{HUNK_UNIT, readString(r)}
}

func (h HunkStr) Type() HunkType {
	return h.htype
}

func (h HunkStr) String() string {
	return fmt.Sprintf("%s '%s'\n", HunkNameMap[h.Type()], h.Name)
}
