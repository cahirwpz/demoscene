package main

import (
	"../misc"
	"image"
	"image/color"
	"log"
)

func DoPalette(img *image.Paletted, params PaletteParams) {
	palette := misc.Palette{}
	colorsCount := len(img.Palette)

	for _, col := range img.Palette {
		r, g, b, a := col.RGBA()
		palette = append(palette, color.RGBA{R: uint8(r), G: uint8(g), B: uint8(b), A: uint8(a)})
	}

	if colorsCount == 0 {
		log.Fatal("Image has no palette!")
	}
	if params.Colors > colorsCount {
		log.Fatalf("Image has %d colors, expected at most %d!", len(palette), params.Colors)
	}
	if params.StoreUnused {
		if params.Colors < colorsCount {
			params.Colors = colorsCount
		}
	}
	err := palette.Export(params.Name)
	if err != nil {
		log.Fatal("Could not export palette to C source.")
	}
	return
}
