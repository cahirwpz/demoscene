package main

import (
	"../misc"
	"flag"
	"fmt"
	"image"
	"image/png"
	"log"
	"os"
)

// static __data_chip u_short _background_bpl[] = {

var printHelp bool

func init() {
	flag.BoolVar(&printHelp, "help", false, "print help message and exit")
}

func dumpPalette(name string, img *image.RGBA) {
	fmt.Printf("u_short %s_pal[] = {\n ", name)
	for x := 0; x < 16; x++ {
		c := img.RGBAAt(x, 0)
		fmt.Printf(" 0x%x%x%x,", c.R>>4, c.G>>4, c.B>>4)
	}
	fmt.Println("\n};\n")
}

func dumpPaletteChanges(name string, img *image.RGBA) {
	fmt.Printf("u_short %s_pchg[] = {\n", name)
	for y := 1; y < img.Bounds().Dy(); y++ {
		fmt.Print(" ")
		bg := img.RGBAAt(0, y)
		for x := 1; x < 16; x++ {
			c := img.RGBAAt(x, y)
			if bg != c {
				fmt.Printf(" 0x%x%x%x%x,", x, c.R>>4, c.G>>4, c.B>>4)
			}
		}
		fmt.Println()
	}
	fmt.Println("};\n")
}

func complementPalette(pal *image.RGBA) {
	width, height := pal.Bounds().Dx(), pal.Bounds().Dy()

	for y := 1; y < height; y++ {
		/* Treat background color as null color. */
		bg := pal.RGBAAt(0, y)

		/* Copy missing palette entries from previous line. */
		for i := 1; i < width; i++ {
			if pal.RGBAAt(i, y) == bg {
				pal.SetRGBA(i, y, pal.RGBAAt(i, y-1))
			}
		}
	}
}

func encodeHAM6(img *image.RGBA, pal *image.RGBA) *image.Paletted {
	width, height := img.Bounds().Dx(), img.Bounds().Dy()

	ham := image.NewPaletted(img.Bounds(), nil)

	for y := 0; y < height; y++ {
		/* Take background color as previous pixel. */
		p := pal.RGBAAt(0, y)

		for x := 0; x < width; x++ {
			c := img.RGBAAt(x, y)
			var v uint8
			/* difference between previous pixel and current */
			dr, dg, db := c.R-p.R, c.G-p.G, c.B-p.B
			if dr >= 0 && dg == 0 && db == 0 {
				v = (c.R >> 4) | 0x20
			} else if dr == 0 && dg >= 0 && db == 0 {
				v = (c.G >> 4) | 0x30
			} else if dr == 0 && dg == 0 && db >= 0 {
				v = (c.B >> 4) | 0x10
			} else {
				for v = 0; v < 16; v++ {
					if c == pal.RGBAAt(int(v), y) {
						break
					}
				}
				if v == 16 {
					log.Fatalf("Problem at pixel (%d, %d), "+
						"color %x%x%x does not match palette!",
						x, y, c.R>>4, c.G>>4, c.B>>4)
				}
			}
			ham.SetColorIndex(x, y, v)
			p = c
		}
	}

	return ham
}

func main() {
	flag.Parse()

	if len(flag.Args()) < 1 || printHelp {
		flag.PrintDefaults()
		os.Exit(1)
	}

	file, err := os.Open(flag.Arg(0))
	defer file.Close()
	if err != nil {
		log.Fatal(err)
	}

	_img, err := png.Decode(file)
	if err != nil {
		log.Fatal(err)
	}

	img, ok := _img.(*image.RGBA)
	if !ok {
		log.Fatal(err)
	}

	name := "face"

	width, height := img.Bounds().Dx()-16, img.Bounds().Dy()
	pal := img.SubImage(image.Rect(0, 0, 16, height)).(*image.RGBA)
	pix := img.SubImage(image.Rect(16, 0, width+16, height)).(*image.RGBA)

	dumpPalette(name, pal)
	dumpPaletteChanges(name, pal)

	complementPalette(pal)
	ham := encodeHAM6(pix, pal)
	bm := misc.NewBitmap(ham)
	err = bm.Export("ham")
	if err != nil {
		log.Fatal(err)
	}
}
