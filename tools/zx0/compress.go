package zx0

import (
	"log"
	"os"
	"os/exec"
)

func Compress(data []byte) []byte {
	var f *os.File
	var fi os.FileInfo
	var err error

	if f, err = os.CreateTemp("", "salvador-*"); err != nil {
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

	cmd := exec.Command("salvador", name, name+".zx0")
	if err := cmd.Run(); err != nil {
		log.Fatalf("%s: %v", cmd.String(), err)
	}

	if f, err = os.Open(name + ".zx0"); err != nil {
		log.Fatal("Open:", err)
	}

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

	defer os.Remove(name + ".zx0")

	return output
}
