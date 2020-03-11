package main

import (
	"encoding/xml"
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

	xmlFile, err := os.Open(flag.Arg(0))
	defer xmlFile.Close()

	if err != nil {
		fmt.Println(err)
	}

	readBytes, err := ioutil.ReadAll(xmlFile)
	if err != nil {
		fmt.Println(err)
	}
	var parsedMap tmx.TiledMap
	xml.Unmarshal(readBytes, &parsedMap)

	b, err := tmx.DecompresString(parsedMap.Layer.Data.Bytes)
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
