package main

import (
	"../iff"
	"../iff/ilbm"
	"flag"
	"image"
	"image/png"
	"log"
	"math"
	"os"
	"path"
)

var printHelp bool

func init() {
	flag.BoolVar(&printHelp, "help", false, "print help message and exit")
}

// Output is interleaved
func c2p(input []byte, width, height, depth int) []byte {
	stride := ((width + 15) &^ 15) / 8
	output := make([]byte, stride*height*depth)

	for y := 0; y < height; y++ {
		for x := 0; x < width; x++ {
			b := input[y*width+x]
			row := output[y*stride*depth : (y+1)*stride*depth]
			for d := 0; d < depth; d++ {
				bit := (b >> d) & 1
				pos := 7 - (x & 7)
				row[stride*d+x/8] |= bit << pos
			}
		}
	}

	return output
}

func loadPNG(name string) image.Image {
	file, err := os.Open(name)
	if err != nil {
		log.Fatal(err)
	}

	img, err := png.Decode(file)
	if err != nil {
		log.Fatal(err)
	}
	file.Close()

	return img
}

func pathWithoutExt(p string) string {
	n := len(p) - len(path.Ext(p))
	return p[:n]
}

func main() {
	flag.Parse()

	if len(flag.Args()) < 1 || printHelp {
		flag.PrintDefaults()
		os.Exit(1)
	}

	img := loadPNG(flag.Arg(0))

	var palette []ilbm.Color
	var ncolors int
	var pixels []byte

	switch img.(type) {
	case *image.Paletted:
		clut := img.(*image.Paletted)
		pixels = clut.Pix
		ncolors = len(clut.Palette)
		palette = make([]ilbm.Color, ncolors)
		for i := 0; i < ncolors; i++ {
			r, g, b, _ := clut.Palette[i].RGBA()
			palette[i] = ilbm.Color{uint8(r >> 8), uint8(g >> 8), uint8(b >> 8)}
		}
	case *image.Gray:
		gray := img.(*image.Gray)
		pixels = gray.Pix
		ncolors = 1
		for p := range pixels {
			if ncolors <= p {
				ncolors = p + 1
			}
		}
		palette = make([]ilbm.Color, ncolors)
		for i := 0; i < ncolors; i++ {
			c := uint8(i * 255.0 / ncolors)
			palette[i] = ilbm.Color{c, c, c}
		}
	default:
		log.Fatal("Image is not 8-bit CLUT or Gray type!")
	}

	width := img.Bounds().Dx()
	height := img.Bounds().Dy()
	depth := int(math.Ceil(math.Log2(float64(ncolors))))

	bmhd := ilbm.BMHD{
		uint16(width), uint16(height), 0, 0, uint8(depth),
		0, 0, 0,
		1, 1, int16(width), int16(height)}
	cmap := ilbm.CMAP{palette}
	body := ilbm.BODY{c2p(pixels, width, height, depth)}

	name := pathWithoutExt(flag.Arg(0))

	f, err := os.Create(name + ".ilbm")
	if err != nil {
		log.Fatal(err)
	}

	chunks := []iff.Chunk{&bmhd, &cmap, &body}
	if err := iff.WriteIff(f, "ILBM", chunks); err != nil {
		log.Fatal("Could not write IFF/ILBM file!")
	}
}
