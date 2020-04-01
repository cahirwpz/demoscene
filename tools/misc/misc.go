package misc

import (
	"image"
	"image/png"
	"log"
	"os"
	"path"
)

func PathWithoutExt(p string) string {
	n := len(p) - len(path.Ext(p))
	return p[:n]
}

func SavePNG(name string, img image.Image) {
	f, err := os.Create(name)
	if err != nil {
		log.Fatal(err)
	}

	if err := png.Encode(f, img); err != nil {
		f.Close()
		log.Fatal(err)
	}

	if err := f.Close(); err != nil {
		log.Fatal(err)
	}
}

func LoadPNG(name string) *image.Paletted {
	file, err := os.Open(name)
	if err != nil {
		log.Fatal(err)
	}

	someImg, err := png.Decode(file)
	if err != nil {
		log.Fatal(err)
	}
	file.Close()

	img, ok := someImg.(*image.Paletted)
	if !ok {
		log.Fatal("Image is not 8-bit CLUT type!")
	}
	return img
}
