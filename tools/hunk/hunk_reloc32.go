package hunk

import "io"

type HunkReloc32 struct {
	reloc map[uint32][]uint32 /* #hunk -> array of offsets */
}

func readHunkReloc32(r io.Reader) (h HunkReloc32) {
	h.reloc = make(map[uint32][]uint32)
	for {
		count := readLong(r)
		if count == 0 {
			break
		}
		hunkRef := readLong(r)
		offsets := make([]uint32, count)
		for i := 0; i < int(count); i++ {
			offsets[i] = readLong(r)
		}
		h.reloc[hunkRef] = offsets
	}
	return
}

func (h HunkReloc32) String() string {
	return "HUNK_RELOC32\n"
}
