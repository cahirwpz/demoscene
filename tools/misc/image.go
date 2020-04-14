package misc

import (
	"image"
)

func GetSubImage(img *image.Paletted, sp image.Point, bounds image.Rectangle) (out *image.Paletted) {
	out = image.NewPaletted(bounds, img.Palette)
	for y := 0; y < bounds.Dy(); y++ {
		for x := 0; x < bounds.Dx(); x++ {
			px := img.ColorIndexAt(sp.X+x, sp.Y+y)
			out.SetColorIndex(x, y, px)
		}
	}
	return
}
