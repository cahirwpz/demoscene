package util

import (
	"bytes"
	"fmt"
	"image"
	"image/color"
	"image/png"
	"log"
	"math"
	"slices"
	"strings"
	"text/template"
)

func CompileTemplate(tpl string, data any) string {
	tmpl, err := template.New("template").Parse(tpl)
	if err != nil {
		log.Panic(err)
	}

	var buf strings.Builder
	err = tmpl.Execute(&buf, data)
	if err != nil {
		log.Panic(err)
	}

	return buf.String()
}

func CutImage(startX, startY, width, height int, img image.Config, pix []uint8) []uint8 {
	offset := img.Width*startY + startX
	out := []uint8{}

	for i := 0; i < height; i++ {
		out = append(out, pix[offset:offset+width]...)
		offset += img.Width
	}

	return out
}

func GetDepth(pix []uint8) int {
	return int(math.Ceil(math.Log2(float64(int(slices.Max(pix)) + 1))))
}

func DecodePNG(file []byte) (image.Image, image.Config, error) {
	cfg, err := png.DecodeConfig(bytes.NewReader(file))
	if err != nil {
		return nil, image.Config{}, fmt.Errorf("expected a PNG image, err: %v", err)
	}

	img, err := png.Decode(bytes.NewReader(file))
	if err != nil {
		return nil, image.Config{}, err
	}

	return img, cfg, nil
}

func Planar(pix []uint8, width, height, depth int, interleaved bool) []uint16 {
	wordsPerRow := (width + 15) / 16

	data := make([]uint16, height*depth*wordsPerRow)

	for y := 0; y < height; y++ {
		row := make([]uint8, (width+15)&-15)
		copy(row, pix[y*width:(y+1)*width])

		for p := 0; p < depth; p++ {
			bits := make([]uint16, len(row))
			// extract bits at position p and put them into the least significant bit
			for i, b := range row {
				bits[i] = uint16((b >> p) & 1)
			}
			// merge them into full words and write to bitmap
			for x := 0; x < width; x = x + 16 {
				var word uint16 = 0
				for i := 0; i < 16; i++ {
					word = word*2 + bits[x+i]
				}
				if interleaved {
					data[(y*depth+p)*wordsPerRow+x/16] = word
				} else {
					data[(p*height+y)*wordsPerRow+x/16] = word
				}
			}
		}
	}

	return data
}

func RGB12(c color.Color) uint {
	r, g, b, _ := c.RGBA() // 16-bit components
	return uint(((r & 0xf000) >> 4) | ((g & 0xf000) >> 8) | ((b & 0xf000) >> 12))
}

func RGB12Lo(c color.Color) uint {
	r, g, b, _ := c.RGBA() // 16-bit components
	return uint((r & 0x0f00) | ((g & 0x0f00) >> 4) | ((b & 0x0f00) >> 8))
}
