package hunk

import "io"

type HunkEnd struct{}

func (h HunkEnd) Type() HunkType {
	return HUNK_END
}

func (h HunkEnd) Write(w io.Writer) {
	writeLong(w, uint32(HUNK_END))
}

func (h HunkEnd) String() string {
	return "HUNK_END\n"
}
