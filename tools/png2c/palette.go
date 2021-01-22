package main

import (
	"../misc"
	"image"
	"image/color"
	"log"
	"strconv"
	"strings"
)

type PaletteParams struct {
	Name        string
	Colors      int
	StoreUnused bool
}

func ParseDoPaletteParams(paletteFlag string) (PaletteParams, error) {
	params := strings.Split(paletteFlag, ",")
	if len(params) < 2 {
		return PaletteParams{}, nil
	}
	colors, err := strconv.Atoi(params[1])
	if err != nil {
		return PaletteParams{}, err
	}
	storeUnused := false
	if len(params) == 3 {
		storeUnused, err = strconv.ParseBool(params[2])
		return PaletteParams{}, err
	}
	return PaletteParams{
		Name:        params[0],
		Colors:      colors,
		StoreUnused: storeUnused,
	}, nil
}

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
