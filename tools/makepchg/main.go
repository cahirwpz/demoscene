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
	"sort"
	"text/template"
)

type paletteReport struct {
	Pixels [][]int
}

func exportReport(baseName string, pixels [][]int) (err error) {
	paletteReportTemplate :=
		` Line | Unique | Used Index
-------------------------------------------------------------------------------
{{- with .Pixels -}}
{{- range $i, $el := .}}
 {{inc $i}} |   {{len $el}}   | {{range $j, $px := $el}}{{$px}}, {{- end}}
{{- end}}
{{- end}}
`
	funcMap := template.FuncMap{
		"inc": func(i int) string {
			return fmt.Sprintf("%4d", i+1)
		},
		"len": func(l []int) string {
			return fmt.Sprintf("%2d", len(l))
		},
	}
	t, err := template.New("export").Funcs(funcMap).Parse(paletteReportTemplate)
	if err != nil {
		return
	}
	pr := paletteReport{pixels}
	file, err := os.Create(baseName + "_report.txt")
	if err != nil {
		return
	}
	defer file.Close()
	err = t.Execute(file, pr)
	if err != nil {
		return
	}
	return
}

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
	pxMap [][]int) (err error) {
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
var txt bool

func init() {
	flag.StringVar(&col, "color", "",
		"Sets palette background, color format 'fad'")
	flag.BoolVar(&txt, "txt", false,
		"Saves report as txt file")
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

	pxMap := make([][]int, 0)

	for y := 0; y < height; y++ {
		rowMap := make(map[uint8]int)
		for x := 0; x < width; x++ {
			px := img.ColorIndexAt(x, y)
			rowMap[px] += 1
		}
		var row []int
		for k := range rowMap {
			row = append(row, int(k))
		}
		sort.Slice(row, func(i, j int) bool {
			return row[i] < row[j]
		})
		pxMap = append(pxMap, row)
	}

	err := exportImage(baseName, img, pxMap)
	if err != nil {
		log.Fatal(err)
	}

	if txt == true {
		err = exportReport(baseName, pxMap)
		if err != nil {
			log.Fatal(err)
		}
	}
}
