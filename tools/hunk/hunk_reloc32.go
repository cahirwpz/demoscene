package hunk

import (
	"io"
	"text/template"
)

type HunkReloc32 struct {
	Reloc map[uint32][]uint32 /* #hunk -> array of offsets */
}

func readHunkReloc32(r io.Reader) (h HunkReloc32) {
	h.Reloc = make(map[uint32][]uint32)
	for {
		count := readLong(r)
		if count == 0 {
			break
		}
		hunkRef := readLong(r)
		offsets := make([]uint32, count)
		for i := 0; i < int(count); i++ {
			offsets[i] = readLong(r)
		}
		h.Reloc[hunkRef] = offsets
	}
	return
}

const (
	sHunkReloc32 = `
HUNK_RELOC32
{{- range $k, $v := .Reloc }}
  {{ $k }} : {{ $v }}
{{- end }}
`
)

var tHunkReloc32 *template.Template

func init() {
	tHunkReloc32 = parseTemplate(sHunkReloc32)
}

func (h HunkReloc32) String() string {
	return executeTemplate(h, tHunkReloc32)
}
