package hunk

import (
	"fmt"
	"io"
	"sort"
	"strings"
)

type HunkDebugGnu struct {
	StabTab    []Stab
	StabStrTab []byte
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

func readStabs(r io.Reader, size uint32) []Stab {
	symtab := make([]Stab, size/12)
	for i := 0; i < int(size)/12; i++ {
		symtab[i] = readStab(r)
	}
	return symtab
}

func readStrTab(r io.Reader, size uint32) []byte {
	strtab := readData(r, size)
	if size&3 != 0 {
		skipBytes(r, int(4-size&3))
	}
	return strtab
}

func readHunkDebugGnu(r io.Reader, name string) HunkDebugGnu {
	var stabTab []Stab
	var stabstrTab []byte
	if name == "" {
		skipBytes(r, 4)
		debugger := readLong(r)
		if debugger != 0x10b {
			panic(fmt.Sprintf("unknown debugger: %08x", debugger))
		}
		stabSize := readLong(r)
		strtabSize := readLong(r)
		stabTab = readStabs(r, stabSize)
		stabstrTab = readStrTab(r, strtabSize)
	}
	if name == ".stab" {
		stabSize := readLong(r) * 4
		stabTab = readStabs(r, stabSize)
	}
	if name == ".stabstr" {
		stabstrSize := readLong(r) * 4
		stabstrTab = readStrTab(r, stabstrSize)
	}
	return HunkDebugGnu{stabTab, stabstrTab}
}

func (h HunkDebugGnu) Type() HunkType {
	return HUNK_DEBUG
}

func (h HunkDebugGnu) String() string {
	var sb strings.Builder

	sb.WriteString("HUNK_DEBUG\n")

	stabstr := parseStringTable(h.StabStrTab)
	if h.StabTab != nil {
		for _, s := range h.StabTab {
			fmt.Fprintf(&sb, " %s\n", s.String(stabstr))
		}
	} else {
		var index []int
		for k, _ := range stabstr {
			index = append(index, k)
		}
		sort.Ints(index)
		for _, i := range index {
			fmt.Fprintf(&sb, " %5d -> %s\n", i, stabstr[i])
		}
	}
	return sb.String()
}
