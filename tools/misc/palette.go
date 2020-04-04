package misc

import (
	"fmt"
	"image/color"
	"os"
	"text/template"
)

const (
	paletteTemplate = `PaletteT {{ name }} = {
  .count = {{ len . }},
  .colors = { {{ range . }}
    {{ color . }},{{ end }}
  }
};

`
)

type Palette []color.RGBA

func (pal *Palette) Export(name string) (err error) {
	funcMap := template.FuncMap{
		"name": func() string { return name },
		"color": func(c color.RGBA) string {
			return fmt.Sprintf("0x%x%x%x", c.R>>4, c.G>>4, c.B>>4)
		}}

	t, err := template.New(name).Funcs(funcMap).Parse(paletteTemplate)
	if err != nil {
		return
	}

	return t.Execute(os.Stdout, pal)
}
