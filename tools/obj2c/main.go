package main

import (
	"flag"
	"fmt"
	"log"
	"os"

	"ghostown.pl/obj2c/obj"
)

var printHelp bool
var scaleFactor float64
var vertexSize int
var edgeSize int
var meshName string
var indexSize int
var offsetX float64
var offsetY float64
var offsetZ float64

func init() {
	flag.BoolVar(&printHelp, "help", false,
		"print help message and exit")
	flag.Float64Var(&offsetX, "offset-x", 0.0,
		"the object will moved in X axis by that amount")
	flag.Float64Var(&offsetY, "offset-y", 0.0,
		"the object will moved in Y axis by that amount")
	flag.Float64Var(&offsetZ, "offset-z", 0.0,
		"the object will moved in Z axis by that amount")
	flag.Float64Var(&scaleFactor, "scale", 1.0,
		"the object will be scaled by this factor")
	flag.StringVar(&meshName, "mesh-name", "",
		"mesh C identifier")
}

func main() {
	flag.Parse()

	if len(flag.Args()) < 2 || printHelp {
		flag.PrintDefaults()
		os.Exit(1)
	}

	if meshName == "" {
		fmt.Println("Mesh name was not provided!")
		os.Exit(1)
	}

	data, err := obj.ParseWavefrontObj(flag.Arg(0))
	if err != nil {
		log.Fatalf("failed to parse file: %v", err)
	}

	cp := obj.ConverterParams{
		Name:    meshName,
		Scale:   scaleFactor,
		OffsetX: offsetX,
		OffsetY: offsetY,
		OffsetZ: offsetZ,
	}

	output, err := obj.Convert(data, cp)
	if err != nil {
		log.Fatalf("failed to convert file: %v", err)
	}

	err = os.WriteFile(flag.Arg(1), []byte(output), 0644)
	if err != nil {
		log.Fatalf("failed to write file %q", flag.Arg(1))
	}
}
