package main

import (
	"flag"
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

	var parsedMap tmx.TiledMap
	parsedMap, err := tmx.ReadFile(flag.Arg(0))
	if err != nil {
		log.Fatal(err)
	}

	b, err := parsedMap.Layer.Data.Decompress()
	if err != nil {
		log.Fatal(err)
	}

	if len(outName) == 0 {
		outName = misc.PathWithoutExt(flag.Arg(0))
	}

	template := parsedMap.GetTiledMapTemplate(outName)

	err = tmx.SaveLayerData(outName, b, template)
	if err != nil {
		log.Fatal(err)
	}

	img := parsedMap.TileSet.Image.ReadSource()

	// TODO Add tiles reordering and optimize.

	misc.SavePNG(outName+"_map.png", img)
}
