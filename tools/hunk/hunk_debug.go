package hunk

import (
	"fmt"
	"io"
	"strings"
)

type HunkDebug struct {
	SymTab []Stab
	StrTab []byte
}

func parseStringTable(data []byte) []string {
	var strtab []string
	var s int = 0
	length := len(data)
	for i := 0; i < length; i++ {
		if data[i] == 0 {
			strtab = append(strtab, string(data[s:i]))
			s = i + 1
		}
	}
	return strtab
}

func readHunkDebug(r io.Reader) (h HunkDebug) {
	/* nlongs := readLong(r) */
	skipBytes(r, 4)
	debugger := readLong(r)
	/*
	 * magic-number: 0x10b
	 * symtabsize strtabsize
	 * symtabdata [length=symtabsize]
	 * strtabdata [length=strtabsize]
	 * [pad bytes]
	 */
	if debugger != 0x10b {
		panic("unknown debugger")
	}
	symtabSize := readLong(r)
	strtabSize := readLong(r)
	for i := 0; i < int(symtabSize); i += 12 {
		h.SymTab = append(h.SymTab, readStab(r))
	}
	skipBytes(r, 4)
	h.StrTab = readData(r, strtabSize)
	if strtabSize&3 != 0 {
		skipBytes(r, int(4-strtabSize&3))
	}
	return
}

func (h HunkDebug) Type() uint32 {
	return HUNK_DEBUG
}

func (h HunkDebug) String() string {
	var sb strings.Builder
	sb.WriteString("HUNK_DEBUG\n")
	for i, s := range h.SymTab {
		fmt.Fprintf(&sb, "%4d: %s\n", i, s)
	}
	sb.WriteString("\n")
	strtab := parseStringTable(h.StrTab)
	for i, s := range strtab {
		fmt.Fprintf(&sb, "%4d: %s\n", i, s)
	}
	return sb.String()
}
