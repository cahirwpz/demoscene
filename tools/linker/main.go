package main

import (
	"flag"
	"ghostown.pl/hunk"
	"os"
)

var printHelp bool

func init() {
	flag.BoolVar(&printHelp, "help", false,
		"print help message and exit")
}

func main() {
	flag.Parse()

	if len(flag.Args()) < 1 || printHelp {
		flag.PrintDefaults()
		os.Exit(1)
	}

	hunks := []hunk.Hunk{}

	for i := 0; i < len(flag.Args()); i++ {
		read, err := hunk.ReadFile(flag.Arg(i))
		if err != nil {
			panic("failed to read Amiga Hunk file: " + flag.Arg(i))
		}
		hunks = append(hunks, read...)
	}

	job := makeLinkJob(hunks)
	checkSymbolRefs(job)
}
