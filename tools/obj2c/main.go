package main

import (
	"flag"
	"log"
	"os"

	"ghostown.pl/obj2c/obj"
)

var printHelp bool
var scaleFactor float64
var calcFaceNormals bool
var calcEdges bool

func init() {
	flag.BoolVar(&printHelp, "help", false,
		"print help message and exit")
	flag.Float64Var(&scaleFactor, "scale", 1.0,
		"the object will be scaled by this factor")
	flag.BoolVar(&calcFaceNormals, "face-normals", false,
		"calculate normal vector to each face")
	flag.BoolVar(&calcEdges, "edges", false,
		"calculate edges and face-to-edge map")

}

func main() {
	flag.Parse()

	if len(flag.Args()) < 2 || printHelp {
		flag.PrintDefaults()
		os.Exit(1)
	}

	object, err := obj.ParseWavefrontObj(flag.Arg(0))
	if err != nil {
		log.Fatalf("failed to parse file: %v", err)
	}

	cp := obj.ConverterParams{
		Scale:       scaleFactor,
		FaceNormals: calcFaceNormals,
		Edges:       calcEdges}

	output, err := obj.Convert(object, cp)
	if err != nil {
		log.Fatalf("failed to convert file: %v", err)
	}

	err = os.WriteFile(flag.Arg(1), []byte(output), 0644)
	if err != nil {
		log.Fatalf("failed to write file %q", flag.Arg(1))
	}
}
