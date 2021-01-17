package hunk

type HunkEnd struct{}

func (h HunkEnd) Type() HunkType {
	return HUNK_END
}

func (h HunkEnd) String() string {
	return "HUNK_END\n"
}
