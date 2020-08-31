package main

import (
	"encoding/binary"
	"flag"
	"fmt"
	"io"
	"os"
)

var printHelp bool

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
)

var HunkName map[uint32]string
var HunkExtName map[uint32]string

func init() {
	HunkName = map[uint32]string{
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

	HunkExtName = map[uint32]string{
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

	flag.BoolVar(&printHelp, "help", false,
		"print help message and exit")
}

func readWord(r io.Reader) (x uint16) {
	if binary.Read(r, binary.BigEndian, &x) != nil {
		panic("no data")
	}
	return
}

func readLong(r io.Reader) (x uint32) {
	if binary.Read(r, binary.BigEndian, &x) != nil {
		panic("no data")
	}
	return
}

func readData(r io.Reader, size uint32) (x []byte) {
	x = make([]byte, size)
	n, err := r.Read(x)
	if err != nil || size != uint32(n) {
		panic("no data")
	}
	return
}

func readString(r io.Reader) (x string) {
	nlongs := readLong(r)
	if nlongs < 1 {
		return ""
	}

	buf := readData(r, nlongs*4)
	return string(buf)
}

func readStringArray(r io.Reader) (x []string) {
	x = make([]string, 1)
	for {
		s := readString(r)
		if s == "" {
			break
		}
		x = append(x, s)
	}
	return
}

type Hunk interface {
	Name() string
}

/* HUNK_HEADER */

type HunkHeader struct {
	residents  []string
	hunks      uint32
	first      uint32
	last       uint32
	specifiers []uint32
}

func (h HunkHeader) Name() string {
	return "HUNK_HEADER"
}

func readHunkHeader(r io.Reader) (h HunkHeader) {
	h.residents = readStringArray(r)
	h.hunks = readLong(r)
	h.first = readLong(r)
	h.last = readLong(r)
	n := h.last - h.first + 1
	h.specifiers = make([]uint32, n)
	for i := 0; i < int(n); i++ {
		h.specifiers[i] = readLong(r)
	}
	return
}

/* HUNK_CODE */

type HunkCode struct {
	code []byte
}

func (h HunkCode) Name() string {
	return "HUNK_CODE"
}

func readHunkCode(r io.Reader) (h HunkCode) {
	nlongs := readLong(r)
	h.code = readData(r, nlongs*4)
	return
}

/* HUNK_DATA */

type HunkData struct {
	data []byte
}

func (h HunkData) Name() string {
	return "HUNK_DATA"
}

func readHunkData(r io.Reader) (h HunkData) {
	nlongs := readLong(r)
	h.data = readData(r, nlongs*4)
	return
}

/* HUNK_BSS */

type HunkBss struct {
	size uint32
}

func (h HunkBss) Name() string {
	return "HUNK_BSS"
}

func readHunkBss(r io.Reader) (h HunkBss) {
	h.size = readLong(r)
	return
}

/* HUNK_RELOC32 */

type HunkReloc32 struct {
	reloc map[uint32][]uint32 /* #hunk -> array of offsets */
}

func (h HunkReloc32) Name() string {
	return "HUNK_RELOC32"
}

func readHunkReloc32(r io.Reader) (h HunkReloc32) {
	h.reloc = make(map[uint32][]uint32)
	for {
		count := readLong(r)
		if count == 0 {
			break
		}
		hunkRef := readLong(r)
		offsets := make([]uint32, count)
		for i := 0; i < int(count); i++ {
			offsets[i] = readLong(r)
		}
		h.reloc[hunkRef] = offsets
	}
	return
}

/* HUNK_SYMBOL */

type Symbol struct {
	name   string
	offset uint32
}

type HunkSymbol struct {
	symbol []Symbol
}

func (h HunkSymbol) Name() string {
	return "HUNK_SYMBOL"
}

func readHunkSymbol(r io.Reader) (h HunkSymbol) {
	for {
		name := readString(r)
		if name == "" {
			break
		}
		offset := readLong(r)
		h.symbol = append(h.symbol, (Symbol{name, offset}))
	}
	return
}

/* HUNK_DEBUG */

type HunkDebug struct {
	data []byte
}

func (h HunkDebug) Name() string {
	return "HUNK_DEBUG"
}

func readHunkDebug(r io.Reader) (h HunkDebug) {
	nlongs := readLong(r)
	h.data = readData(r, nlongs*4)
	return
}

/* HUNK_END */

type HunkEnd struct{}

func (h HunkEnd) Name() string {
	return "HUNK_END"
}

func ReadFile(path string) (err error) {
	file, err := os.Open(path)
	defer file.Close()

	if err != nil {
		return
	}

	var hunk Hunk
	var hunkId uint32

	for {
		err = binary.Read(file, binary.BigEndian, &hunkId)
		if err != nil {
			if err == io.EOF {
				break
			}
			panic("no data")
		}
		switch hunkId {
		case HUNK_HEADER:
			hunk = readHunkHeader(file)
		case HUNK_CODE:
			hunk = readHunkCode(file)
		case HUNK_DATA:
			hunk = readHunkData(file)
		case HUNK_BSS:
			hunk = readHunkBss(file)
		case HUNK_RELOC32:
			hunk = readHunkReloc32(file)
		case HUNK_SYMBOL:
			hunk = readHunkSymbol(file)
		case HUNK_DEBUG:
			hunk = readHunkDebug(file)
		case HUNK_END:
			hunk = HunkEnd{}
		default:
			panic(HunkName[hunkId])
		}
		fmt.Printf("%s\n", hunk.Name())
	}
	return
}

func main() {
	flag.Parse()

	if len(flag.Args()) < 1 || printHelp {
		flag.PrintDefaults()
		os.Exit(1)
	}

	ReadFile(flag.Arg(0))
}
