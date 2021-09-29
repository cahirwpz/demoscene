package ilbm

import (
	"fmt"
	"ghostown.pl/iff"
)

type Color struct {
	R, G, B uint8
}

type CMAP struct {
	Colors []Color
}

func (cmap CMAP) Name() string {
	return "CMAP"
}

func (cmap *CMAP) Read(r iff.Reader) {
	n := int(r.Size() / 3)
	cmap.Colors = make([]Color, n)
	for i := 0; i < n; i++ {
		cmap.Colors[i] = Color{r.ReadU8(), r.ReadU8(), r.ReadU8()}
	}
}

func (cmap CMAP) String() string {
	s := "{"
	sep := ""
	for _, c := range cmap.Colors {
		s += sep
		s += fmt.Sprintf("#%02x%02x%02x", c.R, c.G, c.B)
		sep = " "
	}
	return s + "}"
}

func makeCMAP() iff.Chunk {
	return &CMAP{}
}
