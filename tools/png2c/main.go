package main

import (
	"flag"
	"image"
	"log"
	"os"

	"../misc"
)

var printHelp bool
var palParams PaletteParams

func init() {
	flag.BoolVar(&printHelp, "help", false,
		"print help message and exit")
	flag.Var(&palParams, "palette", "Output Amiga palette 'name,color'")
}

func main() {
	flag.Parse()

	if len(flag.Args()) < 1 || printHelp {
		flag.PrintDefaults()
		os.Exit(1)
	}

	img, ok := misc.LoadPNG(flag.Arg(0)).(*image.Paletted)
	if !ok {
		log.Fatal("Only 8-bit images with palette supported.")
	}
	if (PaletteParams{}) != palParams {
		DoPalette(img, palParams)
	}
}
