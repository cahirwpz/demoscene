package hunk

import (
	"fmt"
	"io"
	"strings"
)

type HunkDebugGnu struct {
	SymTab []Stab
	StrTab []byte
}

func parseStringTable(data []byte) map[int]string {
	strtab := make(map[int]string)
	var s int = 0
	length := len(data)
	for i := 0; i < length; i++ {
		if data[i] == 0 {
			str := string(data[s:i])
			if str != "" {
				strtab[s] = str
			}
			s = i + 1
		}
	}
	return strtab
}

func readHunkDebugGnu(r io.Reader) (h HunkDebugGnu) {
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
		panic(fmt.Sprintf("unknown debugger: %08x", debugger))
	}
	symtabSize := readLong(r)
	strtabSize := readLong(r)
	for i := 0; i < int(symtabSize); i += 12 {
		h.SymTab = append(h.SymTab, readStab(r))
	}
	h.StrTab = readData(r, strtabSize)
	if strtabSize&3 != 0 {
		skipBytes(r, int(4-strtabSize&3))
	}
	return
}

func (h HunkDebugGnu) Type() HunkType {
	return HUNK_DEBUG
}

func (h HunkDebugGnu) String() string {
	var sb strings.Builder

	sb.WriteString("HUNK_DEBUG\n")

	strtab := parseStringTable(h.StrTab)
	for _, s := range h.SymTab {
		fmt.Fprintf(&sb, " %s\n", s.String(strtab))
	}
	return sb.String()
}
