package protracker

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io"
	"log"
)

/*
 * Raw binary representation of Protracker module.
 */

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

type rawModule struct {
	Name    [20]byte
	Samples [31]rawSample
	Length  uint8      // Song length 1-128
	Restart uint8      // Obsolete Noisetracker restart byte
	Song    [128]uint8 // Module patterns, 0-127, holds patterns numbers to play
	Magic   [4]byte    // 1080, 4 bytes - Magic Number
}

func (m rawModule) getName() string {
	return string(bytes.TrimRight(m.Name[:], "\x00"))
}

func (m rawModule) getType() string {
	return string(m.Magic[:])
}

/*
 * In memory representation of Protracker module.
 */

type ChanData struct {
	SampleNumber uint8
	Note         *Note
	Effect       uint8
	EffectParams uint8
}

type Pattern [64][4]ChanData // 64 rows with 4 channels

type Sample struct {
	Name         string
	Data         []byte
	Finetune     uint8
	Volume       uint8
	RepeatOffset uint16
	RepeatLength uint16
}

type Module struct {
	Name     string
	Song     []uint8
	Patterns []Pattern
	Samples  []Sample
}

func readPattern(r io.ReadSeeker) (pattern Pattern) {
	for row := 0; row < 64; row++ {
		for channel := 0; channel < 4; channel++ {
			var data uint32
			err := binary.Read(r, binary.BigEndian, &data)
			if err != nil {
				log.Fatal(err)
			}

			sup := (data & 0xF0000000) >> 28
			per := (data & 0x0FFF0000) >> 16
			slo := (data & 0x0000F000) >> 12
			eff := (data & 0x00000F00) >> 8
			efp := (data & 0x000000FF)

			// Combine sample upper and lower bits
			sample := uint8(sup<<4 | slo)

			pattern[row][channel] = ChanData{
				SampleNumber: sample,
				Note:         PeriodToNote(int(per)),
				Effect:       uint8(eff),
				EffectParams: uint8(efp),
			}
		}
	}

	return pattern
}

func readSample(r io.Reader, rawSample rawSample) Sample {
	length := rawSample.Length * 2
	data := make([]uint8, length)
	n, err := r.Read(data)
	if err != nil {
		log.Fatal(err)
	}
	if n != int(length) {
		log.Fatalf("Read %d bytes (expected %d bytes)!", n, length)
	}
	return Sample{
		Name:         rawSample.getName(),
		Data:         data,
		Finetune:     rawSample.Finetune,
		Volume:       rawSample.Volume,
		RepeatOffset: rawSample.RepeatOffset * 2,
		RepeatLength: rawSample.RepeatLength * 2,
	}
}

func ReadModule(r io.ReadSeeker) Module {
	var mod rawModule
	err := binary.Read(r, binary.BigEndian, &mod)
	if err != nil {
		log.Fatal(err)
	}

	if mod.getType() != "M.K." {
		log.Fatalf("Unknown module type: %s!", mod.getType())
	}

	patternCount := 0
	for _, patNum := range mod.Song {
		if int(patNum) > patternCount {
			patternCount = int(patNum)
		}
	}
	patternCount += 1

	patterns := make([]Pattern, patternCount)
	for i := 0; i < len(patterns); i++ {
		patterns[i] = readPattern(r)
	}

	samples := make([]Sample, 31)
	for i := 0; i < len(samples); i++ {
		samples[i] = readSample(r, mod.Samples[i])
	}

	return Module{
		Name:     mod.getName(),
		Song:     mod.Song[:mod.Length],
		Patterns: patterns,
		Samples:  samples,
	}
}

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
