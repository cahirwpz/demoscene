package main

import (
	"../misc"
	"bytes"
	"encoding/binary"
	"flag"
	"fmt"
	"io"
	"log"
	"os"
	"text/template"
)

var moduleReportTemplate = `Module Name: {{.Name}}
Module Type: {{.Type}}
Patterns Order: [{{- range .PatternsOrder}}{{.}} {{end}}]
{{range $idx, $pat := .PatternsData}}
 ---------------=[ Pattern {{printf "%2d" $idx}} ]=---------------
{{- range $index, $note := $pat.Notes}}
{{- if noteNewLine $note}}
{{noteToString $note}}
{{- else}}
{{- noteToString $note}}
{{- end}}
{{- end}}
{{end}}
 ---------=[ Samples ]=---------
{{range $idx, $sam := .Samples}}
{{- printf "%3d %-22s %5d" $idx $sam.Name (len $sam.Data)}}
{{end}}
`

var PeriodMap = map[uint16]NoteInfo{
	1712: {C, 0},
	1616: {CSharp, 0},
	1525: {D, 0},
	1440: {DSharp, 0},
	1357: {E, 0},
	1281: {F, 0},
	1209: {FSharp, 0},
	1141: {G, 0},
	1077: {GSharp, 0},
	1017: {A, 0},
	961:  {ASharp, 0},
	907:  {B, 0},

	856: {C, 1},
	808: {CSharp, 1},
	762: {D, 1},
	720: {DSharp, 1},
	678: {E, 1},
	640: {F, 1},
	604: {FSharp, 1},
	570: {G, 1},
	538: {GSharp, 1},
	508: {A, 1},
	480: {ASharp, 1},
	453: {B, 1},

	428: {C, 2},
	404: {CSharp, 2},
	381: {D, 2},
	360: {DSharp, 2},
	339: {E, 2},
	320: {F, 2},
	302: {FSharp, 2},
	285: {G, 2},
	269: {GSharp, 2},
	254: {A, 2},
	240: {ASharp, 2},
	226: {B, 2},

	214: {C, 3},
	202: {CSharp, 3},
	190: {D, 3},
	180: {DSharp, 3},
	170: {E, 3},
	160: {F, 3},
	151: {FSharp, 3},
	143: {G, 3},
	135: {GSharp, 3},
	127: {A, 3},
	120: {ASharp, 3},
	113: {B, 3},

	107: {C, 4},
	101: {CSharp, 4},
	95:  {D, 4},
	90:  {DSharp, 4},
	85:  {E, 4},
	80:  {F, 4},
	76:  {FSharp, 4},
	71:  {G, 4},
	67:  {GSharp, 4},
	64:  {A, 4},
	60:  {ASharp, 4},
	57:  {B, 4},
}

/*
 * Raw binary (on disk) representation of Protracker module.
 */

type rawModule struct {
	Name          [20]byte
	Samples       [31]rawSample
	Length        uint8      // Song length 1-128
	Restart       uint8      // Obsolete Noisetracker restart byte
	PatternsOrder [128]uint8 // Module patterns, 0-127, holds patterns numbers to play
	Magic         [4]byte    // 1080, 4 bytes - Magic Number
}

func (s rawModule) getName() string {
	return string(bytes.TrimRight(s.Name[:], "\x00"))
}

type rawNote uint32

func (n rawNote) ToNote() (note NoteData) {
	sup := uint32(0xF0000000) & uint32(n) >> 28
	per := uint32(0x0FFF0000) & uint32(n) >> 16
	slo := uint32(0x0000F000) & uint32(n) >> 12
	eff := uint32(0x00000F00) & uint32(n) >> 8
	efp := uint32(0x000000FF) & uint32(n)

	// Combine sample upper and lower bits
	sample := uint8(sup<<4 | slo)

	var info *NoteInfo
	ni, ok := PeriodMap[uint16(per)]
	if ok {
		info = &ni
	} else {
		if per > 0 {
			log.Fatalf("Period not found %v.", per)
		}
		info = nil
	}

	return NoteData{
		SampleNumber: sample,
		Note:         info,
		Effect:       uint8(eff),
		EffectParams: uint8(efp),
	}
}

type rawSample struct {
	Name         [22]byte
	Length       uint16 // Words. Multiply by two to get length in bytes.
	Finetune     uint8  // Lower 4 bits are the finetune value, stored as a signed four bit number.
	Volume       uint8  // Volume, range is $00-$40, or 0-64 decimal.
	RepeatOffset uint16 // Words. Multiply by two to get offset in bytes.
	RepeatLength uint16 // Words. Multiply by two to get replen in bytes.
}

func (s rawSample) getName() string {
	return string(bytes.TrimRight(s.Name[:], "\x00"))
}

/*
 * Internal representation of Protracker module.
 */

type Module struct {
	Name          string
	Type          string
	Channels      uint8
	PatternsCount uint8
	PatternsOrder []uint8
	PatternsData  []Pattern
	Samples       []Sample
}

type Pattern struct {
	Index int
	Notes []NoteData
}

type NoteData struct {
	SampleNumber uint8
	Note         *NoteInfo
	Effect       uint8
	EffectParams uint8
	Channel      uint8
	Row          uint8
}

func (nd NoteData) ToString() string {
	var note string
	if nd.Note != nil {
		note = nd.Note.ToString()
	} else {
		note = " _ "
	}
	return fmt.Sprintf(" %s %02X %01X%02X ", note,
		nd.SampleNumber, nd.Effect, nd.EffectParams)
}

type NoteInfo struct {
	Note   NoteName
	Octave uint8
}

func (ni NoteInfo) ToString() string {
	return fmt.Sprintf("%s%d", ni.Note.ToString(), ni.Octave)
}

type NoteName uint8

const (
	C NoteName = iota
	CSharp
	D
	DSharp
	E
	F
	FSharp
	G
	GSharp
	A
	ASharp
	B
)

func (n NoteName) ToString() string {
	switch n {
	case C:
		return "C-"
	case CSharp:
		return "C#"
	case D:
		return "D-"
	case DSharp:
		return "D#"
	case E:
		return "E-"
	case F:
		return "F-"
	case FSharp:
		return "F#"
	case G:
		return "G-"
	case GSharp:
		return "G#"
	case A:
		return "A-"
	case ASharp:
		return "A#"
	case B:
		return "B-"
	default:
		panic("Unknown note!")
	}
}

type Sample struct {
	Name         string
	Data         []byte
	Finetune     uint8
	Volume       uint8
	RepeatOffset uint16
	RepeatLength uint16
}

func (m Module) exportReport(baseName string) (err error) {
	funcMap := template.FuncMap{
		"noteNewLine": func(n NoteData) bool { return n.Channel == 0 },
		"noteToString": func(n NoteData) string {
			return n.ToString()
		},
	}
	t, err := template.New("export").Funcs(funcMap).Parse(moduleReportTemplate)
	if err != nil {
		return
	}
	file, err := os.Create(baseName + "_report.txt")
	if err != nil {
		return
	}
	err = t.Execute(file, m)
	file.Close()
	return
}

func readPatterns(highestPattern int, r io.ReadSeeker) (patterns []Pattern, err error) {
	_, err = r.Seek(1084, 0)
	if err != nil {
		return nil, err
	}
	for i := 0; i < highestPattern; i++ {
		var notes []NoteData
		for patRow := 0; patRow < 64; patRow++ {
			notesData := make([]rawNote, 4)
			err = binary.Read(r, binary.BigEndian, &notesData)
			if err != nil {
				return nil, err
			}
			for idx, nd := range notesData {
				note := nd.ToNote()
				note.Channel = uint8(idx)
				note.Row = uint8(patRow)
				notes = append(notes, note)
			}
		}
		patterns = append(patterns, Pattern{Notes: notes, Index: i})
	}
	return patterns, err
}

func modFromReader(r io.ReadSeeker) (mod *Module, err error) {
	var rawMod rawModule

	err = binary.Read(r, binary.BigEndian, &rawMod)
	if err != nil {
		return mod, err
	}

	highestPattern := 0
	for _, i := range rawMod.PatternsOrder {
		if int(i) > highestPattern {
			highestPattern = int(i)
		}
	}

	patterns, err := readPatterns(highestPattern, r)
	if err != nil {
		log.Fatal(err)
	}

	var samples []Sample
	for i := 0; i < 31; i++ {
		var n int
		rawSample := rawMod.Samples[i]
		data := make([]uint8, rawSample.Length*2)
		n, err = r.Read(data)
		if err != nil || n != int(rawSample.Length*2) {
			return nil, err
		}
		sample := Sample{
			Name:         rawSample.getName(),
			Data:         data,
			Finetune:     rawSample.Finetune,
			Volume:       rawSample.Volume,
			RepeatOffset: rawSample.RepeatOffset * 2,
			RepeatLength: rawSample.RepeatLength * 2,
		}
		samples = append(samples, sample)
	}

	mod = &Module{
		Name:          rawMod.getName(),
		Type:          string(rawMod.Magic[:]),
		PatternsOrder: rawMod.PatternsOrder[:rawMod.Length],
		PatternsData:  patterns,
		Samples:       samples,
	}

	return mod, nil
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

	mod, err := modFromReader(file)
	if err != nil {
		log.Fatal(err)
	}
	err = mod.exportReport(baseName)
	if err != nil {
		log.Fatal(err)
	}
}
