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
	EXT        StabType = 0x01
	ABS        StabType = 0x02
	TEXT       StabType = 0x04
	DATA       StabType = 0x06
	BSS        StabType = 0x08
	INDR       StabType = 0x0a
	SIZE       StabType = 0x0c
	COMM       StabType = 0x12
	SETA       StabType = 0x14
	SETT       StabType = 0x16
	SETD       StabType = 0x18
	SETB       StabType = 0x1a
	SETV       StabType = 0x1c
	WARNING    StabType = 0x1e
	FN         StabType = 0x1f
	GSYM       StabType = 0x20
	FNAME      StabType = 0x22
	FUN        StabType = 0x24
	STSYM      StabType = 0x26
	LCSYM      StabType = 0x28
	MAIN       StabType = 0x2a
	ROSYM      StabType = 0x2c
	PC         StabType = 0x30
	NSYMS      StabType = 0x32
	NOMAP      StabType = 0x34
	MAC_DEFINE StabType = 0x36
	OBJ        StabType = 0x38
	MAC_UNDEF  StabType = 0x3a
	OPT        StabType = 0x3c
	RSYM       StabType = 0x40
	SLINE      StabType = 0x44
	DSLINE     StabType = 0x46
	BSLINE     StabType = 0x48
	FLINE      StabType = 0x4c
	EHDECL     StabType = 0x50
	CATCH      StabType = 0x54
	SSYM       StabType = 0x60
	ENDM       StabType = 0x62
	SO         StabType = 0x64
	LSYM       StabType = 0x80
	BINCL      StabType = 0x82
	SOL        StabType = 0x84
	PSYM       StabType = 0xa0
	EINCL      StabType = 0xa2
	ENTRY      StabType = 0xa4
	LBRAC      StabType = 0xc0
	EXCL       StabType = 0xc2
	SCOPE      StabType = 0xc4
	RBRAC      StabType = 0xe0
	BCOMM      StabType = 0xe2
	ECOMM      StabType = 0xe4
	ECOML      StabType = 0xe8
	WITH       StabType = 0xea
	NBTEXT     StabType = 0xf0
	NBDATA     StabType = 0xf2
	NBBSS      StabType = 0xf4
	NBSTS      StabType = 0xf6
	NBLCS      StabType = 0xf8
)

func (st StabType) String() string {
	switch st {
	case UNDF:
		return "UNDF"
	case EXT:
		return "EXT"
	case ABS:
		return "ABS"
	case TEXT:
		return "TEXT"
	case DATA:
		return "DATA"
	case BSS:
		return "BSS"
	case INDR:
		return "INDR"
	case SIZE:
		return "SIZE"
	case COMM:
		return "COMM"
	case SETA:
		return "SETA"
	case SETT:
		return "SETT"
	case SETD:
		return "SETD"
	case SETB:
		return "SETB"
	case SETV:
		return "SETV"
	case WARNING:
		return "WARNING"
	case FN:
		return "FN"
	case GSYM:
		return "GSYM"
	case FNAME:
		return "FNAME"
	case FUN:
		return "FUN"
	case STSYM:
		return "STSYM"
	case LCSYM:
		return "LCSYM"
	case MAIN:
		return "MAIN"
	case ROSYM:
		return "ROSYM"
	case PC:
		return "PC"
	case NSYMS:
		return "NSYMS"
	case NOMAP:
		return "NOMAP"
	case MAC_DEFINE:
		return "MAC_DEFINE"
	case OBJ:
		return "OBJ"
	case MAC_UNDEF:
		return "MAC_UNDEF"
	case OPT:
		return "OPT"
	case RSYM:
		return "RSYM"
	case SLINE:
		return "SLINE"
	case DSLINE:
		return "DSLINE"
	case BSLINE:
		return "BSLINE"
	case FLINE:
		return "FLINE"
	case EHDECL:
		return "EHDECL"
	case CATCH:
		return "CATCH"
	case SSYM:
		return "SSYM"
	case ENDM:
		return "ENDM"
	case SO:
		return "SO"
	case LSYM:
		return "LSYM"
	case BINCL:
		return "BINCL"
	case SOL:
		return "SOL"
	case PSYM:
		return "PSYM"
	case EINCL:
		return "EINCL"
	case ENTRY:
		return "ENTRY"
	case LBRAC:
		return "LBRAC"
	case EXCL:
		return "EXCL"
	case SCOPE:
		return "SCOPE"
	case RBRAC:
		return "RBRAC"
	case BCOMM:
		return "BCOMM"
	case ECOMM:
		return "ECOMM"
	case ECOML:
		return "ECOML"
	case WITH:
		return "WITH"
	case NBTEXT:
		return "NBTEXT"
	case NBDATA:
		return "NBDATA"
	case NBBSS:
		return "NBBSS"
	case NBSTS:
		return "NBSTS"
	case NBLCS:
		return "NBLCS"
	default:
		return "?"
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

func (s Stab) Write(w io.Writer) {
	if binary.Write(w, binary.BigEndian, s.StrOff) != nil {
		panic("failed to write Stab.StrOff")
	}
	if binary.Write(w, binary.BigEndian, s.BinType) != nil {
		panic("failed to write Stab.BinType")
	}
	if binary.Write(w, binary.BigEndian, s.Other) != nil {
		panic("failed to write Stab.Other")
	}
	if binary.Write(w, binary.BigEndian, s.Desc) != nil {
		panic("failed to write Stab.Desc")
	}
	if binary.Write(w, binary.BigEndian, s.Value) != nil {
		panic("failed to write Stab.Value")
	}
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
			s.Type().String(), s.Other, s.Desc, strtab[int(s.StrOff)])
	} else {
		return fmt.Sprintf("%08x %c %6s %04x %02x %d", s.Value, visibility,
			s.Type().String(), s.Other, s.Desc, s.StrOff)
	}
}
