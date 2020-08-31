package hunk

import (
	"fmt"
	"io"
	"sort"
	"strings"
)

type Symbol struct {
	Offset uint32
	Name   string
}

type HunkSymbol struct {
	Symbol []Symbol
}

func readHunkSymbol(r io.Reader) (h HunkSymbol) {
	for {
		name := readString(r)
		if name == "" {
			break
		}
		offset := readLong(r)
		h.Symbol = append(h.Symbol, (Symbol{offset, name}))
	}
	sort.Slice(h.Symbol, func(i, j int) bool {
		return h.Symbol[i].Offset < h.Symbol[j].Offset
	})
	return
}

func (h HunkSymbol) String() string {
	var sb strings.Builder
	sb.WriteString("HUNK_SYMBOL\n")
	for _, s := range h.Symbol {
		fmt.Fprintf(&sb, "  0x%08x %s\n", s.Offset, s.Name)
	}
	return sb.String()
}
