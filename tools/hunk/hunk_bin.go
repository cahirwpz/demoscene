package hunk

import (
	"encoding/hex"
	"fmt"
	"io"
)

type HunkBin struct {
	htype uint32
	Bytes []byte
}

func readHunkCode(r io.Reader) HunkBin {
	return HunkBin{HUNK_CODE, readData(r, readLong(r)*4)}
}

func readHunkData(r io.Reader) HunkBin {
	return HunkBin{HUNK_DATA, readData(r, readLong(r)*4)}
}

func (h HunkBin) Type() uint32 {
	return h.htype
}

func (h HunkBin) String() string {
	return fmt.Sprintf(
		"%s [%d bytes]\n%s", HunkNameMap[h.Type()], len(h.Bytes), hex.Dump(h.Bytes))
}
