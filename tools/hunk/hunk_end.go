package hunk

type HunkEnd struct{}

func (h HunkEnd) Name() string {
	return "HUNK_END"
}
