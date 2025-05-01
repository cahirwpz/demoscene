package ilbm

import (
	"bytes"
	"ghostown.pl/iff"
	"log"
	"text/template"
)

const drngTemplate = `{
  Min: {{.Min}},
  Max: {{.Max}},
  Rate: {{.Rate}},
  Flags: {{if .Flags}} RNG_ACTIVE {{- else}} NONE {{- end}},
  DColor: { {{- range .Dcolor }} {{ .Cell}}: {{ printf "#%02x%02x%02x" .R .G .B}}, {{ end -}} },
  DIndex:  { {{- range .Dindex }} {{.Cell}}: {{.Index}}, {{ end -}} }
}`

type DColor struct {
	Cell uint8
	R    uint8
	G    uint8
	B    uint8
}

type DIndex struct {
	Cell  uint8
	Index uint8
}

type DRNG struct {
	Min    uint8
	Max    uint8
	Rate   int16
	Flags  int16
	Dcolor []DColor
	Dindex []DIndex
}

func (drng DRNG) Name() string {
	return "DRNG"
}

func (drng *DRNG) Read(r iff.Reader) {
	drng.Min = r.ReadU8()
	drng.Max = r.ReadU8()
	drng.Rate = r.ReadI16()
	drng.Flags = r.ReadI16()
	ntrue := r.ReadU8()
	nregs := r.ReadU8()

	drng.Dcolor = make([]DColor, ntrue)
	for i := 0; uint8(i) < ntrue; i++ {
		drng.Dcolor[i].Cell = r.ReadU8()
		drng.Dcolor[i].R = r.ReadU8()
		drng.Dcolor[i].G = r.ReadU8()
		drng.Dcolor[i].B = r.ReadU8()
	}

	drng.Dindex = make([]DIndex, nregs)
	for i := 0; uint8(i) < nregs; i++ {
		drng.Dindex[i].Cell = r.ReadU8()
		drng.Dindex[i].Index = r.ReadU8()
	}
}

func (drng DRNG) String() string {
	t, err := template.New("drng_chunk").Parse(drngTemplate)
	if err != nil {
		log.Fatal(err)
	}

	var out bytes.Buffer
	err = t.Execute(&out, drng)
	if err != nil {
		log.Fatal(err)
	}

	return out.String()
}

func makeDRNG() iff.Chunk {
	return &DRNG{}
}
