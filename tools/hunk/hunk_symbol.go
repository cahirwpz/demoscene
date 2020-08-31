package hunk

import "io"

type Symbol struct {
	name   string
	offset uint32
}

type HunkSymbol struct {
	symbol []Symbol
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

func (h HunkSymbol) String() string {
	return "HUNK_SYMBOL\n"
}
