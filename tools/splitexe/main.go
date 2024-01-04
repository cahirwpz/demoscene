package main

import (
	"flag"
	"fmt"
	"os"
	"slices"

	"ghostown.pl/hunk"
)

var printHelp bool

func init() {
	flag.BoolVar(&printHelp, "help", false,
		"print help message and exit")
}

type Loadable struct {
	Hunks   []hunk.Hunk
	Indices []uint32
}

func splitExe(hunks []hunk.Hunk) []*Loadable {
	var loadables []*Loadable

	header := hunks[0].(*hunk.HunkHeader)

	var loadableHunk uint32 = 0
	seenCodeHunk := false

	var hs []hunk.Hunk
	var is []uint32
	var hdr *hunk.HunkHeader

	for _, h := range hunks[1:] {
		ht := h.Type()

		if ht == hunk.HUNK_CODE {
			if seenCodeHunk {
				loadables = append(loadables, &Loadable{hs, is})
			}

			seenCodeHunk = true
			hdr = &hunk.HunkHeader{}
			hs = []hunk.Hunk{hdr}
			is = []uint32{}
		}

		if ht == hunk.HUNK_CODE || ht == hunk.HUNK_DATA || ht == hunk.HUNK_BSS {
			hs = append(hs, h)
			is = append(is, loadableHunk)

			hdr.Specifiers = append(hdr.Specifiers, header.Specifiers[loadableHunk])
			hdr.Hunks += 1
			hdr.Last = hdr.Hunks - 1

			loadableHunk += 1
		}

		if ht == hunk.HUNK_RELOC32 {
			hs = append(hs, h)
		}
	}

	return append(loadables, &Loadable{hs, is})
}

func writeExe(exeName string, l *Loadable) {
	println(l.Hunks[0].String())

	hs := append(l.Hunks, &hunk.HunkEnd{})

	for _, h := range hs {
		if h.Type() == hunk.HUNK_RELOC32 {
			hr := h.(*hunk.HunkReloc32)
			for i, r := range hr.Relocs {
				newIndex := slices.Index(l.Indices, r.HunkRef)
				if newIndex >= 0 {
					hr.Relocs[i].HunkRef = uint32(newIndex)
				} else {
					hr.Relocs[i].HunkRef += uint32(len(l.Indices))
				}
			}
			hr.Sort()
		}
	}

	if err := hunk.WriteFile(exeName, hs); err != nil {
		panic("failed to write Amiga Hunk file")
	}
}

func main() {
	flag.Parse()

	if len(flag.Args()) < 1 || printHelp {
		flag.PrintDefaults()
		os.Exit(1)
	}

	hunks, err := hunk.ReadFile(flag.Arg(0))
	if err != nil {
		panic("failed to read Amiga Hunk file")
	}

	if hunks[0].Type() != hunk.HUNK_HEADER {
		panic("Not an executable file")
	}

	loadables := splitExe(hunks)
	n := flag.NArg() - 1

	if len(loadables) != n {
		fmt.Printf("Please provide %d executable file names!\n", len(loadables))
		os.Exit(1)
	}

	for i, loadable := range loadables {
		writeExe(flag.Arg(i+1), loadable)
	}
}
