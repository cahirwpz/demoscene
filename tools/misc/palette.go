package misc

import (
	"image/color"
	"os"
	"text/template"
)

const (
	paletteTemplate = `PaletteT {{ name }} = {
  .count = {{ len . }},
  .colors = { {{ range . }}
    { {{.R}}, {{ .G }}, {{ .B }} },{{ end }}
  }
};

`
)

type Palette []color.RGBA

func (pal *Palette) Export(name string) (err error) {
	funcMap := template.FuncMap{
		"name": func() string { return name }}

	t, err := template.New(name).Funcs(funcMap).Parse(paletteTemplate)
	if err != nil {
		return
	}

	return t.Execute(os.Stdout, pal)
}
