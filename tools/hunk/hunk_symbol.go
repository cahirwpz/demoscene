package hunk

import (
	"io"
	"sort"
	"text/template"
)

type Symbol struct {
	Offset uint32
	Name   string
}

type HunkSymbol struct {
	Symbol []Symbol
}

func readHunkSymbol(r io.Reader) (h HunkSymbol) {
	for {
		name := readString(r)
		if name == "" {
			break
		}
		offset := readLong(r)
		h.Symbol = append(h.Symbol, (Symbol{offset, name}))
	}
	sort.Slice(h.Symbol, func(i, j int) bool {
		return h.Symbol[i].Offset < h.Symbol[j].Offset
	})
	return
}

const (
	sHunkSymbol = `
HUNK_SYMBOL
{{- range .Symbol }}
  {{printf "0x%08x" .Offset }} {{ .Name }}
{{- end }}
`
)

var tHunkSymbol *template.Template

func init() {
	tHunkSymbol = parseTemplate(sHunkSymbol)
}

func (h HunkSymbol) String() string {
	return executeTemplate(h, tHunkSymbol)
}
