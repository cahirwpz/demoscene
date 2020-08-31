package hunk

import (
	"strings"
	"text/template"
)

func parseTemplate(s string) (t *template.Template) {
	var err error
	t, err = template.New("export").Parse(s)
	if err != nil {
		panic("parse error")
	}
	return
}

func executeTemplate(h Hunk, t *template.Template) string {
	var buf strings.Builder
	err := t.Execute(&buf, h)
	if err != nil {
		panic("execute error")
	}
	return buf.String()
}
