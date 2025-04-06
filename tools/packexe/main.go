package main

import (
	"bytes"
	"flag"
	"fmt"
	"log"
	"os"

	"ghostown.pl/hunk"
	"ghostown.pl/lzsa"
	"ghostown.pl/zx0"
)

var unpack bool
var printHelp bool
var compAlgoStr string

func init() {
	flag.BoolVar(&unpack, "unpack", false,
		"unpack hunks instead of packing them")
	flag.BoolVar(&printHelp, "help", false,
		"print help message and exit")
	flag.StringVar(&compAlgoStr, "algo", "zx0",
		"default compression algorithm [zx0,lzsa2,lzsa1]")
}

type Action int

const (
	Pack   Action = 0
	Unpack Action = 1
)

type CompAlgo int

const (
	Zx0   CompAlgo = 0
	Lzsa1 CompAlgo = 1
	Lzsa2 CompAlgo = 2
)

var compAlgo CompAlgo

func setCompAlgo(s string) {
	switch s {
	case "zx0":
		compAlgo = Zx0
	case "lzsa2":
		compAlgo = Lzsa2
	case "lzsa1":
		compAlgo = Lzsa1
	default:
		log.Fatalf("Unknown compression algorithm: %s", s)
	}
}

func packData(data []byte, algo CompAlgo) ([]byte, hunk.CompType) {
	var packed []byte
	var comp hunk.CompType

	switch algo {
	case Zx0:
		comp = hunk.COMP_ZX0
		packed = zx0.Compress(data)
	case Lzsa1:
		comp = hunk.COMP_LZSA
		packed = lzsa.Compress(lzsa.V1, data)
	case Lzsa2:
		comp = hunk.COMP_LZSA
		packed = lzsa.Compress(lzsa.V2, data)
	default:
		panic("unknown compression algorithm")
	}

	return packed, comp
}

func unpackData(data []byte, comp hunk.CompType) []byte {
	var unpacked []byte

	switch comp {
	case hunk.COMP_ZX0:
		unpacked = zx0.Decompress(data)
	case hunk.COMP_LZSA:
		unpacked = lzsa.Decompress(data)
	default:
		panic("unknown compression algorithm")
	}

	return unpacked
}

func FileSize(path string) int64 {
	fi, err := os.Stat(path)
	if err != nil {
		log.Fatal("Stat:", err)
	}
	return fi.Size()
}

func packHunk(hd *hunk.HunkBin, hunkNum int, header *hunk.HunkHeader) {
	if hd.Comp != hunk.COMP_NONE {
		fmt.Printf("Hunk %d (%s): already packed\n", hunkNum,
			hd.Type().String())
		return
	}

	fmt.Printf("Compressing hunk %d (%s): %d", hunkNum,
		hd.Type().String(), hd.Data.Len())
	packed, comp := packData(hd.Data.Bytes(), compAlgo)
	fmt.Printf(" -> %d\n", len(packed))

	if len(packed) >= hd.Data.Len() {
		println("Skipping compression...")
		return
	}

	hd.Data = bytes.NewBuffer( packed)
	hd.Comp = comp
	/* add extra 8 bytes for in-place decompression */
	header.Specifiers[hunkNum] += 2
}

func unpackHunk(hd *hunk.HunkBin, hunkNum int, header *hunk.HunkHeader) {
	if hd.Comp == hunk.COMP_NONE {
		fmt.Printf("Hunk %d (%s): already unpacked\n", hunkNum,
			hd.Type().String())
		return
	}

	fmt.Printf("Decompressing hunk %d (%s): %d", hunkNum,
		hd.Type().String(), len(hd.Data.Bytes()))
	unpacked := unpackData(hd.Data.Bytes(), hd.Comp)
	fmt.Printf(" -> %d\n", len(unpacked))
	hd.Data = bytes.NewBuffer(unpacked)
	hd.Comp = 0
	header.Specifiers[hunkNum] -= 2
}

func processExe(hs []hunk.Hunk, action Action) {
	header, ok := hs[0].(*hunk.HunkHeader)
	if !ok {
		panic("Not an executable file")
	}

	loadableNum := 0
	for _, h := range hs {
		switch h.Type() {
		case hunk.HUNK_CODE, hunk.HUNK_DATA:
			hd := h.(*hunk.HunkBin)
			if action == Pack {
				packHunk(hd, loadableNum, header)
			} else {
				unpackHunk(hd, loadableNum, header)
			}
			loadableNum += 1
		case hunk.HUNK_BSS:
			loadableNum += 1
		}
	}
}

func main() {
	flag.Parse()

	if len(flag.Args()) < 1 || printHelp {
		flag.PrintDefaults()
		os.Exit(1)
	}

	setCompAlgo(compAlgoStr)

	hunks, err := hunk.ReadFile(flag.Arg(0))
	if err != nil {
		panic("failed to read Amiga Hunk file")
	}

	beforeSize := FileSize(flag.Arg(0))

	if unpack {
		processExe(hunks, Unpack)
	} else {
		processExe(hunks, Pack)
	}

	outName := flag.Arg(0)

	if len(flag.Args()) > 1 {
		outName = flag.Arg(1)
	}

	if err := hunk.WriteFile(outName, hunks, 0755); err != nil {
		panic("failed to write Amiga Hunk file")
	}

	afterSize := FileSize(outName)

	if !unpack {
		fmt.Printf("%s: %d -> %d (%.2f%% gain)\n", flag.Arg(0), beforeSize,
			afterSize, 100.0*(1.0-float64(afterSize)/float64(beforeSize)))
	}
}
