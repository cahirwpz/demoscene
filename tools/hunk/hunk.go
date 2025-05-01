package hunk

import (
	"io"
)

/* Refer to The AmigaDOS Manual (3rd Edition), chapter 10. */

type HunkType uint32

const (
	HUNK_NONE         HunkType = 0
	HUNK_UNIT         HunkType = 999
	HUNK_NAME         HunkType = 1000
	HUNK_CODE         HunkType = 1001
	HUNK_DATA         HunkType = 1002
	HUNK_BSS          HunkType = 1003
	HUNK_RELOC32      HunkType = 1004
	HUNK_RELOC16      HunkType = 1005
	HUNK_RELOC8       HunkType = 1006
	HUNK_EXT          HunkType = 1007
	HUNK_SYMBOL       HunkType = 1008
	HUNK_DEBUG        HunkType = 1009
	HUNK_END          HunkType = 1010
	HUNK_HEADER       HunkType = 1011
	HUNK_OVERLAY      HunkType = 1013
	HUNK_BREAK        HunkType = 1014
	HUNK_DREL32       HunkType = 1015
	HUNK_DREL16       HunkType = 1016
	HUNK_DREL8        HunkType = 1017
	HUNK_LIB          HunkType = 1018
	HUNK_INDEX        HunkType = 1019
	HUNK_RELOC32SHORT HunkType = 1020
	HUNK_RELRELOC32   HunkType = 1021
	HUNK_ABSRELOC16   HunkType = 1022
)

func (ht HunkType) String() string {
	switch ht {
	case HUNK_UNIT:
		return "HUNK_UNIT"
	case HUNK_NAME:
		return "HUNK_NAME"
	case HUNK_CODE:
		return "HUNK_CODE"
	case HUNK_DATA:
		return "HUNK_DATA"
	case HUNK_BSS:
		return "HUNK_BSS"
	case HUNK_RELOC32:
		return "HUNK_RELOC32"
	case HUNK_RELOC16:
		return "HUNK_RELOC16"
	case HUNK_RELOC8:
		return "HUNK_RELOC8"
	case HUNK_EXT:
		return "HUNK_EXT"
	case HUNK_SYMBOL:
		return "HUNK_SYMBOL"
	case HUNK_DEBUG:
		return "HUNK_DEBUG"
	case HUNK_END:
		return "HUNK_END"
	case HUNK_HEADER:
		return "HUNK_HEADER"
	case HUNK_OVERLAY:
		return "HUNK_OVERLAY"
	case HUNK_BREAK:
		return "HUNK_BREAK"
	case HUNK_DREL32:
		return "HUNK_DREL32"
	case HUNK_DREL16:
		return "HUNK_DREL16"
	case HUNK_DREL8:
		return "HUNK_DREL8"
	case HUNK_LIB:
		return "HUNK_LIB"
	case HUNK_INDEX:
		return "HUNK_INDEX"
	case HUNK_RELOC32SHORT:
		return "HUNK_RELOC32SHORT"
	case HUNK_RELRELOC32:
		return "HUNK_RELRELOC32"
	case HUNK_ABSRELOC16:
		return "HUNK_ABSRELOC16"
	default:
		return "?"
	}
}

type ExtType uint8

const (
	EXT_NONE      ExtType = 255
	EXT_SYMB      ExtType = 0   // symbol table
	EXT_DEF       ExtType = 1   // relocatable definition
	EXT_ABS       ExtType = 2   // Absolute definition
	EXT_RES       ExtType = 3   // no longer supported
	EXT_GNU_LOCAL ExtType = 33  // GNU local symbol definition
	EXT_REF32     ExtType = 129 // 32 bit absolute reference to symbol
	EXT_COMMON    ExtType = 130 // 32 bit absolute reference to COMMON block
	EXT_REF16     ExtType = 131 // 16 bit PC-relative reference to symbol
	EXT_REF8      ExtType = 132 // 8  bit PC-relative reference to symbol
	EXT_DEXT32    ExtType = 133 // 32 bit data relative reference
	EXT_DEXT16    ExtType = 134 // 16 bit data relative reference
	EXT_DEXT8     ExtType = 135 // 8  bit data relative reference
	EXT_RELREF32  ExtType = 136 // 32 bit PC-relative reference to symbol
	EXT_RELCOMMON ExtType = 137 // 32 bit PC-relative reference to COMMON block
	EXT_ABSREF16  ExtType = 138 // 16 bit absolute reference to symbol
	EXT_ABSREF8   ExtType = 139 // 8 bit absolute reference to symbol
)

func (et ExtType) String() string {
	switch et {
	case EXT_SYMB:
		return "EXT_SYMB"
	case EXT_DEF:
		return "EXT_DEF"
	case EXT_ABS:
		return "EXT_ABS"
	case EXT_RES:
		return "EXT_RES"
	case EXT_GNU_LOCAL:
		return "EXT_GNU_LOCAL"
	case EXT_REF32:
		return "EXT_REF32"
	case EXT_COMMON:
		return "EXT_COMMON"
	case EXT_REF16:
		return "EXT_REF16"
	case EXT_REF8:
		return "EXT_REF8"
	case EXT_DEXT32:
		return "EXT_DEXT32"
	case EXT_DEXT16:
		return "EXT_DEXT16"
	case EXT_DEXT8:
		return "EXT_DEXT8"
	case EXT_RELREF32:
		return "EXT_RELREF32"
	case EXT_RELCOMMON:
		return "EXT_RELCOMMON"
	case EXT_ABSREF16:
		return "EXT_ABSREF16"
	case EXT_ABSREF8:
		return "EXT_ABSREF8"
	default:
		return "?"
	}
}

type HunkFlag uint32

const (
	HUNKB_CHIP = 30
	HUNKB_FAST = 31

	HUNKF_PUBLIC HunkFlag = 0
	HUNKF_CHIP   HunkFlag = 1 << HUNKB_CHIP
	HUNKF_FAST   HunkFlag = 1 << HUNKB_FAST
	HUNKF_OTHER  HunkFlag = 3 << HUNKB_CHIP

	HUNKF_FLAG_MASK = uint32(HUNKF_CHIP) | uint32(HUNKF_FAST)
	HUNKF_SIZE_MASK = ^HUNKF_FLAG_MASK
)

func (flag HunkFlag) String() string {
	switch flag & HunkFlag(HUNKF_FLAG_MASK) {
	case HUNKF_PUBLIC:
		return "MEMF_PUBLIC"
	case HUNKF_CHIP:
		return "MEMF_CHIP"
	case HUNKF_FAST:
		return "MEMF_FAST"
	default:
		return "OTHER"
	}
}

func hunkSpec(x uint32) (HunkFlag, uint32) {
	return HunkFlag(x & HUNKF_FLAG_MASK), (x & HUNKF_SIZE_MASK) * 4
}

type DebugType uint32

const (
	DEBUG_HCLN   DebugType = 0x48434c4e
	DEBUG_HEAD   DebugType = 0x48454144
	DEBUG_LINE   DebugType = 0x4c494e45
	DEBUG_ODEF   DebugType = 0x4f444546
	DEBUG_OPTS   DebugType = 0x4f505453
	DEBUG_ZMAGIC DebugType = 267
)

func (dt DebugType) String() string {
	switch dt {
	case DEBUG_HCLN:
		return "HCLN"
	case DEBUG_HEAD:
		return "HEAD"
	case DEBUG_LINE:
		return "LINE"
	case DEBUG_ODEF:
		return "ODEF"
	case DEBUG_OPTS:
		return "OPTS"
	case DEBUG_ZMAGIC:
		return "ZMAGIC"
	default:
		return "?"
	}
}

type Hunk interface {
	String() string
	Type() HunkType
	Write(w io.Writer)
}
