package main

import (
	"bytes"
	"flag"
	"fmt"
	"log"
	"os"
	"os/exec"

	"ghostown.pl/hunk"
)

var unpack bool
var printHelp bool

func init() {
	flag.BoolVar(&unpack, "unpack", false,
		"unpack hunks instead of packing them")
	flag.BoolVar(&printHelp, "help", false,
		"print help message and exit")
}

type Action int

const (
	Pack   Action = 0
	Unpack Action = 1
)

func FileSize(path string) int64 {
	fi, err := os.Stat(path)
	if err != nil {
		log.Fatal("Stat:", err)
	}
	return fi.Size()
}

func zx0(data []byte, action Action) []byte {
	var f *os.File
	var fi os.FileInfo
	var err error
	var dir string

	if dir, err = os.Getwd(); err != nil {
		log.Fatal("Getwd:", err)
	}

	if f, err = os.CreateTemp(dir, flag.Arg(0)); err != nil {
		log.Fatal("CreateTemp:", err)
	}

	name := f.Name()

	defer os.Remove(name)

	if _, err := f.Write(data); err != nil {
		log.Fatal("Write:", err)
	}
	if err = f.Close(); err != nil {
		log.Fatal("Close:", err)
	}

	var cmd *exec.Cmd
	if action == Pack {
		cmd = exec.Command("salvador", name, name+".zx0")
	} else {
		cmd = exec.Command("salvador", "-d", name, name+".zx0")
	}

	if err := cmd.Run(); err != nil {
		log.Fatalf("%s: %v", cmd.String(), err)
	}

	if f, err = os.Open(name + ".zx0"); err != nil {
		log.Fatal("Open:", err)
	}

	if fi, err = f.Stat(); err != nil {
		log.Fatal("Stat:", err)
	}

	output := make([]byte, fi.Size())
	if _, err = f.Read(output); err != nil {
		log.Fatal("Read:", err)
	}
	if err = f.Close(); err != nil {
		log.Fatal("Close:", err)
	}

	defer os.Remove(name + ".zx0")

	return output
}

func packHunk(hd *hunk.HunkBin, hunkNum int, header *hunk.HunkHeader) {
	if hd.Flags == hunk.HUNKF_OTHER {
		fmt.Printf("Hunk %d (%s): already packed\n", hunkNum,
			hd.Type().String())
		return
	}

	fmt.Printf("Compressing hunk %d (%s): %d", hunkNum,
		hd.Type().String(), hd.Data.Len())
	packed := zx0(hd.Data.Bytes(), Pack)
	fmt.Printf(" -> %d\n", len(packed))
	if len(packed) >= hd.Data.Len() {
		println("Skipping compression...")
	} else {
		hd.Data = bytes.NewBuffer(packed)
		hd.Flags = hunk.HUNKF_OTHER
		header.Specifiers[hunkNum] += 1
	}
}

func unpackHunk(hd *hunk.HunkBin, hunkNum int, header *hunk.HunkHeader) {
	if hd.Flags != hunk.HUNKF_OTHER {
		fmt.Printf("Hunk %d (%s): already unpacked\n", hunkNum,
			hd.Type().String())
		return
	}

	fmt.Printf("Decompressing hunk %d (%s): %d", hunkNum,
		hd.Type().String(), hd.Data.Len())
	unpacked := zx0(hd.Data.Bytes(), Unpack)
	hd.Data = bytes.NewBuffer(unpacked)
	fmt.Printf(" -> %d\n", hd.Data.Len())
	hd.Flags = 0
	header.Specifiers[hunkNum] -= 1
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
		fmt.Printf("Compressed: %d -> %d (%.2f%% gain)\n", beforeSize, afterSize,
			100.0*(1.0-float64(afterSize)/float64(beforeSize)))
	}
}
