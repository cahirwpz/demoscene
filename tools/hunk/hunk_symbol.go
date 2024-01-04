package hunk

import (
	"fmt"
	"io"
	"sort"
	"strings"
)

type Symbol struct {
	Name  string
	Value uint32
}

type HunkSymbol struct {
	Symbols []Symbol
}

func readHunkSymbol(r io.Reader) *HunkSymbol {
	var symbols []Symbol
	for {
		name := readString(r)
		if name == "" {
			break
		}
		value := readLong(r)
		symbols = append(symbols, Symbol{name, value})
	}
	return &HunkSymbol{symbols}
}

func (h *HunkSymbol) Sort() {
	sort.Slice(h.Symbols, func(i, j int) bool {
		return h.Symbols[i].Value < h.Symbols[j].Value
	})
}

func (h HunkSymbol) Type() HunkType {
	return HUNK_SYMBOL
}

func (h HunkSymbol) Write(w io.Writer) {
	writeLong(w, uint32(HUNK_SYMBOL))
	for _, s := range h.Symbols {
		writeStringWithSize(w, s.Name)
		writeLong(w, s.Value)
	}
	writeLong(w, 0)
}

func (h HunkSymbol) String() string {
	var sb strings.Builder
	sb.WriteString("HUNK_SYMBOL\n")
	for _, s := range h.Symbols {
		fmt.Fprintf(&sb, "  0x%08x %s\n", s.Value, s.Name)
	}
	return sb.String()
}
