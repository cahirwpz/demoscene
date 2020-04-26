package main

import (
	"../misc"
	"flag"
	"fmt"
	"image"
	"image/color"
	"image/draw"
	"log"
	"sort"
)

func parseHexColor(s string) (c color.RGBA, err error) {
	c.A = 0xff
	if len(s) == 3 {
		_, err = fmt.Sscanf(s, "%1x%1x%1x", &c.R, &c.G, &c.B)
		c.R *= 17
		c.G *= 17
		c.B *= 17
	} else {
		err = fmt.Errorf("invalid color length, must be 3")
	}
	return
}

func exportImage(baseName string, img *image.Paletted,
	pxMap map[int][]uint8) (err error) {
	var rowMax int
	for _, row := range pxMap {
		if len(row) > rowMax {
			rowMax = len(row)
		}
	}

	dim := image.Rect(0, 0, rowMax, img.Bounds().Dy())
	palImg := image.NewRGBA(dim)
	fill := image.NewRGBA(image.Rect(0, 0, palImg.Bounds().Dx()+1,
		img.Bounds().Dy()))
	fillCol := color.RGBA{0, 255, 0, 255}
	if len(col) != 0 {
		fillCol, err = parseHexColor(col)
		if err != nil {
			return err
		}
	}
	draw.Draw(fill, image.Rect(0, 0, fill.Bounds().Dx(),
		fill.Bounds().Dy()),
		&image.Uniform{C: fillCol},
		image.Point{},
		draw.Over)

	for y := 0; y < palImg.Bounds().Dy(); y++ {
		rowLen := len(pxMap[y])
		for x := 0; x < palImg.Bounds().Dx(); x++ {
			var px color.Color
			if x < rowLen {
				px = img.Palette[pxMap[y][x]]
			} else {
				px = fillCol
			}
			fill.Set(x+1, y, px)
		}
	}

	outDim := image.Rect(0, 0,
		img.Bounds().Dx()+fill.Bounds().Dx(), img.Bounds().Dy())
	out := image.NewRGBA(outDim)

	draw.Draw(out,
		image.Rect(img.Bounds().Dx(), 0, img.Bounds().Dx()+fill.Bounds().
			Dx(), img.Bounds().Dy()), fill, image.Point{}, draw.Over)
	draw.Draw(out, image.Rect(0, 0,
		img.Bounds().Dx(), img.Bounds().Dy()),
		img, image.Point{}, draw.Over)

	misc.SavePNG(baseName+"_pal.png", out)

	return
}

var col string

func init() {
	flag.StringVar(&col, "color", "",
		"Sets palette background, color format 'fad'")
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
