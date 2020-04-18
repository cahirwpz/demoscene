package misc

import (
	"fmt"
	"image/color"
	"os"
	"text/template"
)

const (
	pchgTemplate = `const int {{ name }}_count = {{ .Count }};

uint16_t {{ name }}[] = {
{{ range . }}  {{ len . }}, {{ range . }}{{ color . }}, {{ end}}
{{ end -}}
};

`
)

type PaletteChanges [][]color.RGBA

func (pchg PaletteChanges) Count() (count int) {
	for _, row := range pchg {
		count += len(row)
	}
	return
}

func (pchg *PaletteChanges) Export(name string) (err error) {
	funcMap := template.FuncMap{
		"name": func() string { return name },
		"color": func(c color.RGBA) string {
			return fmt.Sprintf("0x%x%x%x%x", c.R>>4, c.G>>4, c.B>>4, c.A)
		}}

	t, err := template.New(name).Funcs(funcMap).Parse(pchgTemplate)
	if err != nil {
		return
	}

	return t.Execute(os.Stdout, pchg)
}
