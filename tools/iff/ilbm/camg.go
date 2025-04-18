package ilbm

import (
	"fmt"
	"ghostown.pl/iff"
)

type CAMG struct {
	mode uint32
}

func (camg CAMG) Name() string {
	return "CAMG"
}

func (camg *CAMG) Read(r iff.Reader) {
	camg.mode = r.ReadU32()
}

func (camg CAMG) String() string {
	return fmt.Sprintf("0x%08x", camg.mode)
}

func makeCAMG() iff.Chunk {
	return &CAMG{}
}
