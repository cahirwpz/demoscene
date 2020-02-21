package ilbm

import ".."
import "fmt"

type BMHD struct {
	Width       uint16
	Height      uint16
	XOrigin     int16
	YOrigin     int16
	NumPlanes   uint8
	Mask        uint8
	Compression uint8
	TransClr    uint16
	XAspect     uint8
	YAspect     uint8
	PageWidth   int16
	PageHeight  int16
}

func (bmhd BMHD) Name() string {
	return "BMHD"
}

func (bmhd *BMHD) Read(r iff.Reader) {
	bmhd.Width = r.ReadU16()
	bmhd.Height = r.ReadU16()
	bmhd.XOrigin = r.ReadI16()
	bmhd.YOrigin = r.ReadI16()
	bmhd.NumPlanes = r.ReadU8()
	bmhd.Mask = r.ReadU8()
	bmhd.Compression = r.ReadU8()
	r.Skip(1)
	bmhd.TransClr = r.ReadU16()
	bmhd.XAspect = r.ReadU8()
	bmhd.YAspect = r.ReadU8()
	bmhd.PageWidth = r.ReadI16()
	bmhd.PageHeight = r.ReadI16()
}

func (bmhd BMHD) String() string {
	return fmt.Sprintf("{Size: %dx%d, Origin: (%d,%d), NumPlanes: %d}",
		bmhd.Width, bmhd.Height, bmhd.XOrigin, bmhd.YOrigin, bmhd.NumPlanes)
}

func makeBMHD() iff.Chunk {
	return &BMHD{}
}
