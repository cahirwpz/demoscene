package main

import (
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"os"

	"../tmx"
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

	//TODO Do something with ungzipped bytes

	fmt.Println(b)

	f, err := os.Create("./out.png")
	if err != nil {
		log.Fatal(err)
	}

	if err := ioutil.WriteFile(f.Name(), []byte(b), 0644); err != nil {
		f.Close()
		log.Fatal(err)
	}
}
