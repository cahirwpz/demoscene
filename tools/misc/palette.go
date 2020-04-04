package misc

import (
	"image/color"
	"os"
	"text/template"
)

const (
	paletteTemplate = `
PaletteT {{.Name}} = {
  .count = {{.Count}},
  .colors = {
		{{with .Colors}}{{ range . }}
		{{.}},
		{{ end }}{{ end }}
	}
};`
)

type Palette struct {
	Colors []color.RGBA
}

func (pal Palette) Count() int {
	return len(pal.Colors)
}

func (pal *Palette) Export(name string) (err error) {
	t, err := template.New(name).Parse(paletteTemplate)
	if err != nil {
		return
	}

	return t.Execute(os.Stdout, pal)
}
