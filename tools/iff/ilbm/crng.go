package ilbm

import (
	"bytes"
	"log"
	"ghostown.pl/iff"
	"text/template"
)

const crngTemplate = `{
  Rate: {{.Rate}},
  Flags: {{if .Flags}}RNG_ACTIVE{{- else}}NONE{{- end}},
  Low: {{.Low}},
  High: {{.High}},
}`

type CRNG struct {
	Rate  int16
	Flags int16
	Low   uint8
	High  uint8
}

func (crng CRNG) Name() string {
	return "CRNG"
}

func (crng *CRNG) Read(r iff.Reader) {
	r.Skip(2)
	crng.Rate = r.ReadI16()
	crng.Flags = r.ReadI16()
	crng.Low = r.ReadU8()
	crng.High = r.ReadU8()
}

func (crng CRNG) String() string {
	t, err := template.New("crng_chunk").Parse(crngTemplate)
	if err != nil {
		log.Fatal(err)
	}

	var out bytes.Buffer
	err = t.Execute(&out, crng)
	if err != nil {
		log.Fatal(err)
	}

	return out.String()
}

func makeCRNG() iff.Chunk {
	return &CRNG{}
}
