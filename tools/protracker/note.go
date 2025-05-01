package protracker

import (
	"fmt"
	"log"
)

type NoteName int

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

var noteNameMap = map[NoteName]string{
	C:      "C",
	CSharp: "C#",
	D:      "D",
	DSharp: "D#",
	E:      "E",
	F:      "F",
	FSharp: "F#",
	G:      "G",
	GSharp: "G#",
	A:      "A",
	ASharp: "A#",
	B:      "B",
}

func (n NoteName) String() (s string) {
	s, ok := noteNameMap[n]
	if !ok {
		log.Fatalf("Unknown note name: %v!", n)
	}
	return
}

type Note struct {
	Name   NoteName
	Octave uint8
}

func (n Note) String() string {
	var sep string
	if len(n.Name.String()) == 1 {
		sep = "-"
	} else {
		sep = ""
	}
	return fmt.Sprintf("%s%s%d", n.Name, sep, n.Octave)
}

var periodNoteMap = map[int]Note{
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

func PeriodToNote(period int) *Note {
	if period == 0 {
		return nil
	}
	n, ok := periodNoteMap[period]
	if !ok {
		log.Fatalf("Unknown period: %v!", period)
	}
	return &n
}
