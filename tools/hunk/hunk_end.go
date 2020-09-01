package hunk

type HunkEnd struct{}

func (h HunkEnd) Type() uint32 {
	return HUNK_END
}

func (h HunkEnd) String() string {
	return "HUNK_END\n"
}
