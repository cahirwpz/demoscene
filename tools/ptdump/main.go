package main

import (
	"flag"
	"fmt"
	"ghostown.pl/misc"
	"log"
	"os"
	"text/template"
)

var moduleReportTemplate = `Module Name: {{.Name}}
Patterns Order: [{{- range .Song}}{{.}} {{end}}]

 ---------=[ Samples ]=---------
{{range $idx, $sam := .Samples}}
{{- printf "%3d %-22s %5d" $idx $sam.Name (len $sam.Data)}}
{{end}}
{{- range $idx, $pat := .Patterns}}
 ---------------=[ Pattern {{printf "%2d" $idx}} ]=---------------
{{range $pat}}
{{- range .}}{{.}}{{- end}}
{{end}}
{{- end}}
`

func (cd ChanData) String() string {
	var note string
	if cd.Note != nil {
		note = cd.Note.String()
	} else {
		note = " _ "
	}
	return fmt.Sprintf(" %s %02X %01X%02X ", note, cd.SampleNumber, cd.Effect,
		cd.EffectParams)
}

func dumpModule(m Module, baseName string) {
	t, err := template.New("export").Parse(moduleReportTemplate)
	if err != nil {
		log.Fatal(err)
	}
	file, err := os.Create(baseName + "_report.txt")
	if err != nil {
		log.Fatal(err)
	}
	err = t.Execute(file, m)
	file.Close()
}

func main() {
	flag.Parse()

	if len(flag.Args()) == 0 {
		os.Exit(1)
	}

	file, err := os.Open(flag.Arg(0))
	if err != nil {
		log.Fatal(err)
	}

	baseName := misc.PathWithoutExt(flag.Arg(0))

	dumpModule(ReadModule(file), baseName)
}
