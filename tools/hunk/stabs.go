package hunk

import (
	"encoding/binary"
	"fmt"
	"io"
)

/* http://sourceware.org/gdb/current/onlinedocs/stabs/Stab-Types.html */

type StabType uint8

const (
	UNDF       StabType = 0x00
	EXT                 = 0x01
	ABS                 = 0x02
	TEXT                = 0x04
	DATA                = 0x06
	BSS                 = 0x08
	INDR                = 0x0a
	SIZE                = 0x0c
	COMM                = 0x12
	SETA                = 0x14
	SETT                = 0x16
	SETD                = 0x18
	SETB                = 0x1a
	SETV                = 0x1c
	WARNING             = 0x1e
	FN                  = 0x1f
	GSYM                = 0x20
	FNAME               = 0x22
	FUN                 = 0x24
	STSYM               = 0x26
	LCSYM               = 0x28
	MAIN                = 0x2a
	ROSYM               = 0x2c
	PC                  = 0x30
	NSYMS               = 0x32
	NOMAP               = 0x34
	MAC_DEFINE          = 0x36
	OBJ                 = 0x38
	MAC_UNDEF           = 0x3a
	OPT                 = 0x3c
	RSYM                = 0x40
	SLINE               = 0x44
	DSLINE              = 0x46
	BSLINE              = 0x48
	FLINE               = 0x4c
	EHDECL              = 0x50
	CATCH               = 0x54
	SSYM                = 0x60
	ENDM                = 0x62
	SO                  = 0x64
	LSYM                = 0x80
	BINCL               = 0x82
	SOL                 = 0x84
	PSYM                = 0xa0
	EINCL               = 0xa2
	ENTRY               = 0xa4
	LBRAC               = 0xc0
	EXCL                = 0xc2
	SCOPE               = 0xc4
	RBRAC               = 0xe0
	BCOMM               = 0xe2
	ECOMM               = 0xe4
	ECOML               = 0xe8
	WITH                = 0xea
	NBTEXT              = 0xf0
	NBDATA              = 0xf2
	NBBSS               = 0xf4
	NBSTS               = 0xf6
	NBLCS               = 0xf8
)

var StabTypeMap map[StabType]string

func init() {
	StabTypeMap = map[StabType]string{
		UNDF:       "UNDF",
		EXT:        "EXT",
		ABS:        "ABS",
		TEXT:       "TEXT",
		DATA:       "DATA",
		BSS:        "BSS",
		INDR:       "INDR",
		SIZE:       "SIZE",
		COMM:       "COMM",
		SETA:       "SETA",
		SETT:       "SETT",
		SETD:       "SETD",
		SETB:       "SETB",
		SETV:       "SETV",
		WARNING:    "WARNING",
		FN:         "FN",
		GSYM:       "GSYM",
		FNAME:      "FNAME",
		FUN:        "FUN",
		STSYM:      "STSYM",
		LCSYM:      "LCSYM",
		MAIN:       "MAIN",
		ROSYM:      "ROSYM",
		PC:         "PC",
		NSYMS:      "NSYMS",
		NOMAP:      "NOMAP",
		MAC_DEFINE: "MAC_DEFINE",
		OBJ:        "OBJ",
		MAC_UNDEF:  "MAC_UNDEF",
		OPT:        "OPT",
		RSYM:       "RSYM",
		SLINE:      "SLINE",
		DSLINE:     "DSLINE",
		BSLINE:     "BSLINE",
		FLINE:      "FLINE",
		EHDECL:     "EHDECL",
		CATCH:      "CATCH",
		SSYM:       "SSYM",
		ENDM:       "ENDM",
		SO:         "SO",
		LSYM:       "LSYM",
		BINCL:      "BINCL",
		SOL:        "SOL",
		PSYM:       "PSYM",
		EINCL:      "EINCL",
		ENTRY:      "ENTRY",
		LBRAC:      "LBRAC",
		EXCL:       "EXCL",
		SCOPE:      "SCOPE",
		RBRAC:      "RBRAC",
		BCOMM:      "BCOMM",
		ECOMM:      "ECOMM",
		ECOML:      "ECOML",
		WITH:       "WITH",
		NBTEXT:     "NBTEXT",
		NBDATA:     "NBDATA",
		NBBSS:      "NBBSS",
		NBSTS:      "NBSTS",
		NBLCS:      "NBLCS",
	}
}

type Stab struct {
	StrOff  int32
	BinType uint8
	Other   int8
	Desc    int16
	Value   uint32
}

func (s Stab) External() bool {
	return bool(s.BinType&1 == 1)
}

func (s Stab) Type() StabType {
	return StabType(s.BinType & 254)
}

func readStab(r io.Reader) (s Stab) {
	if binary.Read(r, binary.BigEndian, &s.StrOff) != nil {
		panic("no data")
	}
	if binary.Read(r, binary.BigEndian, &s.BinType) != nil {
		panic("no data")
	}
	if binary.Read(r, binary.BigEndian, &s.Other) != nil {
		panic("no data")
	}
	if binary.Read(r, binary.BigEndian, &s.Desc) != nil {
		panic("no data")
	}
	if binary.Read(r, binary.BigEndian, &s.Value) != nil {
		panic("no data")
	}
	return
}

func (s Stab) String(strtab map[int]string) string {
	var visibility rune

	if s.External() {
		visibility = 'g'
	} else {
		visibility = 'l'
	}

	if strtab != nil {
		return fmt.Sprintf("%08x %c %6s %04x %02x %s", s.Value, visibility,
			StabTypeMap[s.Type()], s.Other, s.Desc, strtab[int(s.StrOff)])
	} else {
		return fmt.Sprintf("%08x %c %6s %04x %02x %d", s.Value, visibility,
			StabTypeMap[s.Type()], s.Other, s.Desc, s.StrOff)
	}
}
