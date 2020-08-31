package hunk

type HunkEnd struct{}

func (h HunkEnd) String() string {
	return "HUNK_END\n"
}
