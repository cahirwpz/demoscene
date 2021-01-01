package hunk

/* Refer to The AmigaDOS Manual (3rd Edition), chapter 10. */

const (
	HUNK_UNIT         = 999
	HUNK_NAME         = 1000
	HUNK_CODE         = 1001
	HUNK_DATA         = 1002
	HUNK_BSS          = 1003
	HUNK_RELOC32      = 1004
	HUNK_RELOC16      = 1005
	HUNK_RELOC8       = 1006
	HUNK_EXT          = 1007
	HUNK_SYMBOL       = 1008
	HUNK_DEBUG        = 1009
	HUNK_END          = 1010
	HUNK_HEADER       = 1011
	HUNK_OVERLAY      = 1013
	HUNK_BREAK        = 1014
	HUNK_DREL32       = 1015
	HUNK_DREL16       = 1016
	HUNK_DREL8        = 1017
	HUNK_LIB          = 1018
	HUNK_INDEX        = 1019
	HUNK_RELOC32SHORT = 1020
	HUNK_RELRELOC32   = 1021
	HUNK_ABSRELOC16   = 1022
)

const (
	EXT_SYMB      = 0   // symbol table
	EXT_DEF       = 1   // relocatable definition
	EXT_ABS       = 2   // Absolute definition
	EXT_RES       = 3   // no longer supported
	EXT_REF32     = 129 // 32 bit absolute reference to symbol
	EXT_COMMON    = 130 // 32 bit absolute reference to COMMON block
	EXT_REF16     = 131 // 16 bit PC-relative reference to symbol
	EXT_REF8      = 132 // 8  bit PC-relative reference to symbol
	EXT_DEXT32    = 133 // 32 bit data relative reference
	EXT_DEXT16    = 134 // 16 bit data relative reference
	EXT_DEXT8     = 135 // 8  bit data relative reference
	EXT_RELREF32  = 136 // 32 bit PC-relative reference to symbol
	EXT_RELCOMMON = 137 // 32 bit PC-relative reference to COMMON block
	EXT_ABSREF16  = 138 // 16 bit absolute reference to symbol
	EXT_ABSREF8   = 139 // 8 bit absolute reference to symbol
)

const (
	/* Any hunks that have the HUNKB_ADVISORY bit set will be ignored if they
	 * aren't understood.  When ignored, they're treated like HUNK_DEBUG hunks.
	 * NOTE: this handling of HUNKB_ADVISORY started as of V39 dos.library!  If
	 * lading such executables is attempted under <V39 dos, it will fail with a
	 * bad hunk type. */
	HUNKB_ADVISORY = 29
	HUNKB_CHIP     = 30
	HUNKB_FAST     = 31

	HUNKF_ADVISORY = 1 << HUNKB_ADVISORY
	HUNKF_CHIP     = 1 << HUNKB_CHIP
	HUNKF_FAST     = 1 << HUNKB_FAST
	HUNKF_MASK     = HUNKF_ADVISORY | HUNKF_CHIP | HUNKF_FAST
)

var HunkNameMap map[uint32]string
var HunkExtNameMap map[uint32]string

func init() {
	HunkNameMap = map[uint32]string{
		HUNK_UNIT:         "HUNK_UNIT",
		HUNK_NAME:         "HUNK_NAME",
		HUNK_CODE:         "HUNK_CODE",
		HUNK_DATA:         "HUNK_DATA",
		HUNK_BSS:          "HUNK_BSS",
		HUNK_RELOC32:      "HUNK_RELOC32",
		HUNK_RELOC16:      "HUNK_RELOC16",
		HUNK_RELOC8:       "HUNK_RELOC8",
		HUNK_EXT:          "HUNK_EXT",
		HUNK_SYMBOL:       "HUNK_SYMBOL",
		HUNK_DEBUG:        "HUNK_DEBUG",
		HUNK_END:          "HUNK_END",
		HUNK_HEADER:       "HUNK_HEADER",
		HUNK_OVERLAY:      "HUNK_OVERLAY",
		HUNK_BREAK:        "HUNK_BREAK",
		HUNK_DREL32:       "HUNK_DREL32",
		HUNK_DREL16:       "HUNK_DREL16",
		HUNK_DREL8:        "HUNK_DREL8",
		HUNK_LIB:          "HUNK_LIB",
		HUNK_INDEX:        "HUNK_INDEX",
		HUNK_RELOC32SHORT: "HUNK_RELOC32SHORT",
		HUNK_RELRELOC32:   "HUNK_RELRELOC32",
		HUNK_ABSRELOC16:   "HUNK_ABSRELOC16",
	}

	HunkExtNameMap = map[uint32]string{
		EXT_SYMB:      "EXT_SYMB",
		EXT_DEF:       "EXT_DEF",
		EXT_ABS:       "EXT_ABS",
		EXT_RES:       "EXT_RES",
		EXT_REF32:     "EXT_REF32",
		EXT_COMMON:    "EXT_COMMON",
		EXT_REF16:     "EXT_REF16",
		EXT_REF8:      "EXT_REF8",
		EXT_DEXT32:    "EXT_DEXT32",
		EXT_DEXT16:    "EXT_DEXT16",
		EXT_DEXT8:     "EXT_DEXT8",
		EXT_RELREF32:  "EXT_RELREF32",
		EXT_RELCOMMON: "EXT_RELCOMMON",
		EXT_ABSREF16:  "EXT_ABSREF16",
		EXT_ABSREF8:   "EXT_ABSREF8",
	}
}

type Hunk interface {
	String() string
	Type() uint32
}
