package hunk

import (
	"io"
)

type HunkExt struct {
	SymDef []SymbolDef
	SymRef []SymbolRef
}

type SymbolRef struct {
	Name string
	Size uint32
	Refs []uint32
}

func readHunkExt(r io.Reader) HunkExt {
	var symdef []SymbolDef
	var symref []SymbolRef
	for {
		nlongs := readLong(r)
		if nlongs == 0 {
			break
		}
		length := nlongs & 0xffffff
		extType := nlongs >> 24
		name := readStringOfSize(r, length)
		switch extType {
		case EXT_DEF, EXT_ABS, EXT_RES:
			value := readLong(r)
			symdef = append(symdef, SymbolDef{name, value})
		case EXT_REF32, EXT_REF16, EXT_REF8, EXT_DEXT32, EXT_DEXT16, EXT_DEXT8:
			symref = append(symref, SymbolRef{name, 0, readArrayOfLong(r)})
		case EXT_COMMON:
			size := readLong(r)
			symref = append(symref, SymbolRef{name, size, readArrayOfLong(r)})
		default:
			panic("unknown external type")
		}
	}
	return HunkExt{}
}

func (h HunkExt) Type() uint32 {
	return HUNK_EXT
}

func (h HunkExt) String() string {
	return "HUNK_EXT\n"
}
