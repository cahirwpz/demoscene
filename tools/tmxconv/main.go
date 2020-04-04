package main

import (
	"flag"
	"image"
	"log"
	"os"

	"../misc"
	"../tmx"
)

var printHelp bool
var outName string

func init() {
	flag.BoolVar(&printHelp, "help", false, "print help message and exit\n")
	flag.StringVar(&outName, "name", "", "set optional output name")
}

func main() {
	flag.Parse()

	if len(flag.Args()) < 1 || printHelp {
		flag.PrintDefaults()
		os.Exit(1)
	}

	if len(outName) == 0 {
		outName = misc.PathWithoutExt(flag.Arg(0))
	}

	var parsedMap tmx.TiledMap
	parsedMap, err := tmx.ReadFile(flag.Arg(0))
	if err != nil {
		log.Fatal(err)
	}

	exportData, err := parsedMap.GetTiledMapExportData(outName)
	if err != nil {
		log.Fatal(err)
	}

	err = exportData.SaveLayerData()
	if err != nil {
		log.Fatal(err)
	}

	img := misc.LoadPNG(parsedMap.TileSet.Image.Source).(*image.Paletted)

	// TODO Add tiles reordering and optimize.

	misc.SavePNG(outName+"_map.png", img)
}
