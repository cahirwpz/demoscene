package main

import (
	"flag"
	"ghostown.pl/iff"
	"ghostown.pl/iff/ilbm"
	"path/filepath"
	"text/template"
	"strings"
	"log"
	"os"
)

type Cycling struct {
	Name string
	Cells [][]int
	Colors [][]int
	Cycles []Cycle
	NColors int
	NCells int
}

type Cycle struct {
	Rate int
	Step int
	NColors int
	CellsIdx int
	ColorsIdx int
}

var printHelp bool
var outputPath, structName string

func init() {
	flag.StringVar(&outputPath, "o", "", "Output filename with processed data")
	flag.StringVar(&structName, "name", "", "Custom structure name")
	flag.BoolVar(&printHelp, "help", false, "print help message and exit")
}

var cTemplate = `
static u_short _{{ .Name }}_colors[{{ .NColors }}] = {
{{- range $i, $val := .Colors }}
  /* cycle {{ $i }} */
  {{- range $val }}
  {{ printf "0x%03x" . }},{{ end }}
{{ end -}}
};

static short _{{ .Name }}_cells[{{ .NCells }}] = {
{{- range $i, $val := .Cells }}
  /* cycle {{ $i }} */
  {{- range $val }}
  {{ . }},{{ end }}
{{ end -}}
};

static __data ColorCyclingT {{ .Name }}_cycling[{{ len .Cycles }}] = {
{{ range .Cycles }}  {
    .rate = {{ printf "0x%04x" .Rate }},
    .step = {{ .Step }},
    .ncolors = {{ .NColors }},
    .cells = &_{{ $.Name }}_cells[{{ .CellsIdx }}],
    .colors = &_{{ $.Name }}_colors[{{ .ColorsIdx }}],
  },
{{ end -}}
};
`

func (data *Cycling) Export() string {
	tmpl, err := template.New("template").Parse(cTemplate)
	if err != nil {
		log.Panic(err)
	}

	var buf strings.Builder
	err = tmpl.Execute(&buf, *data)
	if err != nil {
		log.Panic(err)
	}

	return buf.String()
}

func rgb12(c ilbm.Color) int {
	return ((int(c.R) & 0xf0) << 4) | (int(c.G) & 0xf0) | ((int(c.B) & 0xf0) >> 4)
}

func interpolate(c0 ilbm.Color, c1 ilbm.Color, step float64) ilbm.Color {
	r := int(c0.R) + int((float64(c1.R) - float64(c0.R)) * step)
	g := int(c0.G) + int((float64(c1.G) - float64(c0.G)) * step)
	b := int(c0.B) + int((float64(c1.B) - float64(c0.B)) * step)
	return ilbm.Color{R: uint8(r), G: uint8(g), B: uint8(b)}
}

func ReadIff(name string) (iff.File, error) {
	file, err := os.Open(name)
	if err != nil {
		return nil, err
	}
	form, err := iff.ReadIff(file)
	file.Close()
	return form, err
}

func RateToSec(rate int16) float64 {
  return 16384.0 / float64(rate) * 1.0 / 60.0
}

func Convert(file iff.File) Cycling {
	var cmap []ilbm.Color

	cycling := Cycling{
		Name: structName,
		NColors: 0,
		NCells: 0,
	}

	for _, chunk := range file.Chunks() {
		if chunk.Name() == "CMAP" {
			cmap = chunk.(*ilbm.CMAP).Colors
		}

		if chunk.Name() == "CRNG" {
			crng := chunk.(*ilbm.CRNG)

      rate := RateToSec(crng.Rate)
			ncolors := int(crng.High - crng.Low + 1)

		  cycling.Cycles = append(cycling.Cycles, Cycle{
				Rate: int(rate * 65536.0),
				Step: 0,
				NColors: ncolors,
				CellsIdx: cycling.NCells,
				ColorsIdx: cycling.NColors,
			})

			colors := make([]int, ncolors)
			for i := 0; i < ncolors; i++ {
				colors[i] = rgb12(cmap[i + int(crng.Low)])
			}

			cycling.Colors = append(cycling.Colors, colors)
			cycling.NColors += len(colors)

			var cells []int
			for i := 0; i < ncolors; i++ {
				cells = append(cells, i + int(crng.Low), i)
			}
			cells = append(cells, -1)

			cycling.Cells = append(cycling.Cells, cells)
			cycling.NCells += len(cells)
		}

		if chunk.Name() == "DRNG" {
			drng := chunk.(*ilbm.DRNG)

      rate := RateToSec(drng.Rate)
			ncolors := int(drng.Max - drng.Min + 1)

		  cycling.Cycles = append(cycling.Cycles, Cycle{
				Rate: int(rate * 65536.0),
				Step: 0,
				NColors: ncolors,
				CellsIdx: cycling.NCells,
				ColorsIdx: cycling.NColors,
			})

			colors := make([]int, ncolors)

			for _, index := range drng.Dindex {
				cell, index := int(index.Cell), int(index.Index)
				colors[cell] = rgb12(cmap[index])
			}

			for j := 1; j < len(drng.Dindex); j++ {
				i0 := drng.Dindex[j-1].Index
				c0 := drng.Dindex[j-1].Cell
				i1 := drng.Dindex[j].Index
				c1 := drng.Dindex[j].Cell
				for k := c0 + 1; k < c1; k++ {
					color := interpolate(cmap[i0], cmap[i1], float64(k - c0) / float64(c1 - c0))
					colors[k] = rgb12(color)
				}
			}

			cycling.Colors = append(cycling.Colors, colors)
			cycling.NColors += len(colors)

			regs := make(map[int]int)
			for _, index := range drng.Dindex {
				cell, index := int(index.Cell), int(index.Index)
				_, found := regs[index]
				if !found {
					regs[index] = cell
				}
			}

			var cells []int
			for reg, cell := range regs {
				cells = append(cells, reg, cell)
			}
			cells = append(cells, -1)

			cycling.Cells = append(cycling.Cells, cells)
			cycling.NCells += len(cells)
		}
	}

	return cycling
}

func main() {
	flag.Parse()

	if len(flag.Args()) < 1 || printHelp {
		flag.PrintDefaults()
		os.Exit(1)
	}

	iffFile, err := ReadIff(flag.Arg(0))
	if err != nil {
		log.Fatal("Error while reading IFF file: ", err)
	}

	if structName == "" {
		baseName := filepath.Base(flag.Arg(0))
    structName = strings.TrimSuffix(baseName, filepath.Ext(baseName))
	}

	cycling := Convert(iffFile)
	output := cycling.Export()

	if outputPath == "" {
		print(output)
	} else {
		outFile, err := os.Create(outputPath)
		if err != nil {
			log.Fatal(err)
		}
		defer outFile.Close()
		outFile.WriteString(output)
		if err != nil {
			log.Fatal(err)
		}
	}
}
