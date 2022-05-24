package main

import (
	"flag"
	"fmt"
	"ghostown.pl/iff"
	_ "ghostown.pl/iff/ilbm"
	"log"
	"os"
)

var printHelp bool

func init() {
	flag.BoolVar(&printHelp, "help", false, "print help message and exit")
}

func ReadIff(name string) (iff.File, error) {
	file, err := os.Open(name)
	if err != nil {
		return nil, err
	}
	form, err := iff.ReadIff(file)
	file.Close()
	return form, err
}

func main() {
	flag.Parse()

	if len(flag.Args()) < 1 || printHelp {
		flag.PrintDefaults()
		os.Exit(1)
	}

	file, err := ReadIff(flag.Arg(0))
	if err != nil {
		log.Fatal("Error while reading IFF file: ", err)
	}

	fmt.Printf("[%s]\n", file.Name())

	for _, chunk := range file.Chunks() {
		fmt.Printf(" [%s] %s\n", chunk.Name(), chunk)
	}
}
