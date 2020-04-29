package main

import (
	"../misc"
	"flag"
	"fmt"
	"image"
	"image/color"
	"image/draw"
	"log"
	"os"
	"text/template"
)

var paletteReportTemplate = ` Line | Unique | Used Index
-------------------------------------------------------------------------------
{{- with . -}}
{{- range $i, $el := .}}
 {{inc $i}} |   {{len $el}}   | {{range $j, $px := $el}}{{$px}}, {{- end}}
{{- end}}
{{- end}}
`

func exportReport(baseName string, pixels [][]int) (err error) {
	funcMap := template.FuncMap{
		"inc": func(i int) string { return fmt.Sprintf("%4d", i+1) },
		"len": func(l []int) string { return fmt.Sprintf("%2d", len(l)) },
	}
	t, err := template.New("export").Funcs(funcMap).Parse(paletteReportTemplate)
	if err != nil {
		return
	}
	file, err := os.Create(baseName + "_report.txt")
	if err != nil {
		return
	}
	err = t.Execute(file, pixels)
	file.Close()
	return
}

func exportImage(baseName string, img *image.Paletted, pxMap [][]int) {
	var rowMax int
	for _, row := range pxMap {
		if len(row) > rowMax {
			rowMax = len(row)
		}
	}

	dim := image.Rect(0, 0, rowMax, img.Bounds().Dy())
	palImg := image.NewRGBA(dim)
	fillDim := image.Rect(0, 0, palImg.Bounds().Dx()+1, img.Bounds().Dy())
	fill := image.NewRGBA(fillDim)
	draw.Draw(fill, image.Rect(0, 0, maxN,
		fill.Bounds().Dy()),
		&image.Uniform{C: bgCol.rgb},
		image.Point{},
		draw.Over)

	for y := 0; y < palImg.Bounds().Dy(); y++ {
		rowLen := len(pxMap[y])
		for x := 0; x < palImg.Bounds().Dx(); x++ {
			var px color.Color
			if x < rowLen {
				px = img.Palette[pxMap[y][x]]
			} else if x >= maxN-1 {
				px = maxCol.rgb
			}
			if px != nil {
				fill.Set(x+1, y, px)
			}
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

type Color struct {
	rgb color.RGBA
}

func (col *Color) String() string {
	c := col.rgb
	return fmt.Sprintf("%x%x%x", c.R/17, c.G/17, c.B/17)
}

func (col *Color) Set(s string) (err error) {
	var c color.RGBA
	c.A = 0xff
	if len(s) == 3 {
		_, err = fmt.Sscanf(s, "%1x%1x%1x", &c.R, &c.G, &c.B)
		c.R *= 17
		c.G *= 17
		c.B *= 17
	} else {
		err = fmt.Errorf("invalid color format must be 3 hex digits")
	}
	col.rgb = c
	return
}

var bgCol = Color{color.RGBA{0, 255, 0, 255}}
var maxCol = Color{color.RGBA{255, 0, 0, 255}}
var report bool
var maxN int

func init() {
	flag.BoolVar(&report, "report", false,
		"Saves report as txt file")
	flag.Var(&bgCol, "bg-color",
		"Sets palette background, 12-bit hex color format 'fad'")
	flag.Var(&maxCol, "max-color",
		"Sets palette background, for color count higher than 'n' ("+
			"default: 16), color format 'fad'")
	flag.IntVar(&maxN, "n", 16,
		"Sets max pixel count for normal background. (default: 16)")
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

	usedPerRow := make([][]int, height)

	for y := 0; y < height; y++ {
		histogram := make([]int, 256)
		for x := 0; x < width; x++ {
			px := img.ColorIndexAt(x, y)
			histogram[px] += 1
		}

		used := make([]int, 0)
		for i, n := range histogram {
			if n > 0 {
				used = append(used, i)
			}
		}

		usedPerRow[y] = used
	}

	exportImage(baseName, img, usedPerRow)

	if report {
		err := exportReport(baseName, usedPerRow)
		if err != nil {
			log.Fatal(err)
		}
	}
}
