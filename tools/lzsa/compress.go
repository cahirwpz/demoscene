package lzsa

import (
	"fmt"
	"log"
	"os"
	"os/exec"
)

func Compress(format Format, data []byte) []byte {
	var f *os.File
	var fi os.FileInfo
	var err error

	lzsaVer := "1"
	if format == V2 {
		lzsaVer = "2"
	}
	lzsaStr := "lzsa" + lzsaVer

	if f, err = os.CreateTemp("", lzsaStr+"-*"); err != nil {
		log.Fatal("CreateTemp:", err)
	}

	name := f.Name()

	defer os.Remove(name)

	if _, err := f.Write(data); err != nil {
		log.Fatal("Write:", err)
	}
	if err = f.Close(); err != nil {
		log.Fatal("Close:", err)
	}

	outName := fmt.Sprintf("%s.%s", name, lzsaStr)

	cmd := exec.Command("lzsa", "-f", lzsaVer, name, outName)
	if err := cmd.Run(); err != nil {
		log.Fatalf("%s: %v", cmd.String(), err)
	}

	if f, err = os.Open(outName); err != nil {
		log.Fatal("Open:", err)
	}

	defer os.Remove(outName)

	if fi, err = f.Stat(); err != nil {
		log.Fatal("Stat:", err)
	}

	output := make([]byte, fi.Size())
	if _, err = f.Read(output); err != nil {
		log.Fatal("Read:", err)
	}
	if err = f.Close(); err != nil {
		log.Fatal("Close:", err)
	}

	return output
}
