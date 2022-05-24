package hunk

import (
	"fmt"
	"io"
	"sort"
	"strings"
)

type SymbolDef struct {
	Name  string
	Value uint32
}

type HunkSymbol struct {
	Symbol []SymbolDef
}

func readHunkSymbol(r io.Reader) HunkSymbol {
	var symbols []SymbolDef
	for {
		name := readString(r)
		if name == "" {
			break
		}
		value := readLong(r)
		symbols = append(symbols, SymbolDef{name, value})
	}
	sort.Slice(symbols, func(i, j int) bool {
		return symbols[i].Value < symbols[j].Value
	})
	return HunkSymbol{symbols}
}

func (h HunkSymbol) Type() HunkType {
	return HUNK_SYMBOL
}

func (h HunkSymbol) String() string {
	var sb strings.Builder
	sb.WriteString("HUNK_SYMBOL\n")
	for _, s := range h.Symbol {
		fmt.Fprintf(&sb, "  0x%08x %s\n", s.Value, s.Name)
	}
	return sb.String()
}
