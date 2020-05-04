package main

import (
	"../misc"
	"bytes"
	"encoding/binary"
	"flag"
	"io"
	"log"
	"os"
	"text/template"
)

var moduleReportTemplate = `Module Name: {{.Name}}
`

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

type rawSample struct {
	Name         [22]byte
	Length       uint16 // Words. Multiply by two to get length in bytes.
	Finetune     uint8  // Lower 4 bits are the finetune value, stored as a signed four bit number.
	Volume       uint8  // Volume, range is $00-$40, or 0-64 decimal.
	RepeatOffset uint16 // Words. Multiply by two to get offset in bytes.
	RepeatLength uint16 // Words. Multiply by two to get replen in bytes.
}

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
	Data  []PatternData
}

type PatternData struct {
	SampleNumber uint8
	Note         NoteData
	Effect       uint8
	EffectParams uint8
	Channel      uint8
	Row          uint8
}

type NoteData struct {
	Note   uint8
	Octave uint8
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
	t, err := template.New("export").Parse(moduleReportTemplate)
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

func modFromReader(r io.ReadSeeker) (mod Module, err error) {
	var rawMod rawModule

	err = binary.Read(r, binary.BigEndian, &rawMod)
	if err != nil {
		return mod, err
	}

	// TODO Implement other stuff
	mod.Name = rawMod.getName()
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
