package main

import (
	"../hunk"
	"flag"
	"fmt"
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

	hunks, err := hunk.ReadFile(flag.Arg(0))
	if err != nil {
		panic("failed to read Amiga Hunk file")
	}

	for _, hunk := range hunks {
		fmt.Printf("%s\n", hunk.Name())
	}
}
