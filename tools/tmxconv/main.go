package main

import (
	"flag"
	"log"
	"os"

	"../tmx"
	"../utils"
)

var printHelp bool

func init() {
	flag.BoolVar(&printHelp, "help", false, "print help message and exit")
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

	baseName := utils.PathWithoutExt(flag.Arg(0))

	err = tmx.SaveLayerData(baseName, b)
	if err != nil {
		log.Fatal(err)
	}

	img := parsedMap.TileSet.Image.ReadSource()

	// TODO Add tiles reordering and optimize.

	tmx.SavePNG(baseName+"_map.png", img)

	err = parsedMap.SaveTiledMapInfo(baseName)
	if err != nil {
		log.Fatal(err)
	}
}
