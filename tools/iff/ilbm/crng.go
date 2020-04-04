package ilbm

import ".."
import "fmt"

type CRNG struct {
	rate  int16
	flags int16
	low   uint8
	high  uint8
}

func (crng CRNG) Name() string {
	return "CRNG"
}

func (crng *CRNG) Read(r iff.Reader) {
	r.Skip(2)
	crng.rate = r.ReadI16()
	crng.flags = r.ReadI16()
	crng.low = r.ReadU8()
	crng.high = r.ReadU8()
}

func (crng CRNG) String() string {
	return fmt.Sprintf("?")
}

func makeCRNG() iff.Chunk {
	return &CRNG{}
}
