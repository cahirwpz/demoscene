package main

import (
	"../misc"
	"flag"
	"image"
	"image/draw"
	"log"
	"sort"
)

func exportImage(baseName string, img *image.Paletted,
	pxMap map[int][]uint8) (err error) {
	var rowMax int
	for _, row := range pxMap {
		if len(row) > rowMax {
			rowMax = len(row)
		}
	}

	dim := image.Rect(0, 0, rowMax, img.Bounds().Dy())
	palImg := image.NewPaletted(dim, img.Palette)
	for y := 0; y < palImg.Bounds().Dy(); y++ {
		for x := 0; x < palImg.Bounds().Dx(); x++ {
			rowLen := len(pxMap[y])
			var px uint8
			if x > rowLen-1 {
				px = 0
			} else {
				px = pxMap[y][x]
			}
			palImg.SetColorIndex(x, y, px)
		}
	}

	outDim := image.Rect(0, 0,
		img.Bounds().Dx()+rowMax, img.Bounds().Dy())
	out := image.NewPaletted(outDim, img.Palette)

	draw.Draw(out,
		image.Rect(img.Bounds().Dx(), 0, img.Bounds().Dx()+palImg.Bounds().Dx(),
			img.Bounds().Dy()), palImg, image.Point{}, draw.Over)
	draw.Draw(out, image.Rect(0, 0,
		img.Bounds().Dx(), img.Bounds().Dy()),
		img, image.Point{}, draw.Over)

	misc.SavePNG(baseName+"_pal.png", out)

	return
}

func main() {
	flag.Parse()

	img, ok := misc.LoadPNG(flag.Arg(0)).(*image.Paletted)
	if !ok {
		log.Fatal("Image is not 8-bit CLUT type!")
	}

	baseName := misc.PathWithoutExt(flag.Arg(0))

	width := img.Bounds().Dx()
	height := img.Bounds().Dy()

	pxMap := make(map[int][]uint8)

	for y := 0; y < height; y++ {
		row := make(map[uint8]int)
		for x := 0; x < width; x++ {
			px := img.ColorIndexAt(x, y)
			row[px] += 1
		}
		for k := range row {
			pxMap[y] = append(pxMap[y], k)
		}
		sort.Slice(pxMap[y], func(i, j int) bool {
			return pxMap[y][i] < pxMap[y][j]
		})
	}

	err := exportImage(baseName, img, pxMap)
	if err != nil {
		log.Fatal(err)
	}
}
