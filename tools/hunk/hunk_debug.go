package hunk

import (
	"bufio"
	"bytes"
	"fmt"
	"io"
	"sort"
	"strings"
)

type HunkDebugGnu struct {
	Name       string
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

func dumpStabTab(stabtab []Stab) []byte {
	var bb bytes.Buffer
	w := bufio.NewWriter(&bb)

	for _, stab := range stabtab {
		stab.Write(w)
	}

	w.Flush()

	return bb.Bytes()
}

func readStrTab(r io.Reader, size uint32) []byte {
	strtab := readData(r, size)
	if size&3 != 0 {
		skipBytes(r, int(4-size&3))
	}
	return strtab
}

func readHunkDebugGnu(r io.Reader, name string) *HunkDebugGnu {
	var stabTab []Stab
	var stabstrTab []byte
	switch name {
	case "":
		skipBytes(r, 4)
		debugger := DebugType(readLong(r))
		if debugger != DEBUG_ZMAGIC {
			panic(fmt.Sprintf("unknown debugger: %08x", debugger))
		}
		stabSize := readLong(r)
		strtabSize := readLong(r)
		stabTab = readStabs(r, stabSize)
		stabstrTab = readStrTab(r, strtabSize)
	case ".stab":
		stabSize := readLong(r) * 4
		stabTab = readStabs(r, stabSize)
	case ".stabstr":
		stabstrSize := readLong(r) * 4
		stabstrTab = readStrTab(r, stabstrSize)
	}
	return &HunkDebugGnu{name, stabTab, stabstrTab}
}

func (h HunkDebugGnu) Type() HunkType {
	return HUNK_DEBUG
}

func (h HunkDebugGnu) Write(w io.Writer) {
	writeLong(w, uint32(HUNK_DEBUG))
	stab := dumpStabTab(h.StabTab)
	switch h.Name {
	case "":
		writeLong(w, 3+bytesSize(stab)+bytesSize(h.StabStrTab))
		writeLong(w, uint32(DEBUG_ZMAGIC))
		writeLong(w, uint32(len(stab)))
		writeLong(w, uint32(len(h.StabStrTab)))
		writeData(w, stab)
		writeData(w, h.StabStrTab)
	case ".stab":
		writeDataWithSize(w, stab)
	case ".stabstr":
		writeDataWithSize(w, h.StabStrTab)
	}
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
		for k := range stabstr {
			index = append(index, k)
		}
		sort.Ints(index)
		for _, i := range index {
			fmt.Fprintf(&sb, " %5d -> %s\n", i, stabstr[i])
		}
	}
	return sb.String()
}
