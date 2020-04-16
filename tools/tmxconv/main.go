package main

import (
	"flag"
	"image"
	"image/draw"
	"log"
	"os"
	"sort"
	"text/template"

	"../misc"
	"../tmx"
)

type cTileMap struct {
	Name        string
	TileWidth   int
	TileHeight  int
	TilesCount  int
	LayerWidth  int
	LayerHeight int
	LayerData   []uint32
}

const (
	cTileMapTemplate = `
static TileSetT {{.Name}}_tiles = {
	.width = {{ .TileWidth}},
	.height = {{ .TileHeight}},
	.count = {{ .TilesCount}},
	.ptrs = NULL
};
const int {{.Name}}_map_width = {{.LayerWidth}};
const int {{.Name}}_map_height = {{.LayerHeight}};
short {{.Name}}_map[] = { 
{{- with .LayerData}}
	{{- range $i, $el := .}}
		{{- if endLine $i }}
			{{ $el}},
		{{- else}}
			{{- $el}},
		{{- end}}
	{{- end}}
{{- end}}
};`
)

func uniqueUint32(ints []uint32) []uint32 {
	keys := make(map[uint32]bool)
	var list []uint32
	for _, entry := range ints {
		if _, value := keys[entry]; !value {
			keys[entry] = true
			list = append(list, entry)
		}
	}
	return list
}

func indexOfUint32(element uint32, data []uint32) int {
	for k, v := range data {
		if element == v {
			return k
		}
	}
	return -1 //not found.
}

func exportTiledMap(tm tmx.TiledMap, name string) (err error) {

	layer, err := tm.Layer.Data.Decode()
	if err != nil {
		return
	}
	img := misc.LoadPNG(tm.TileSet.Image.Source).(*image.Paletted)

	unique := uniqueUint32(layer)
	sort.Slice(unique, func(i, j int) bool {
		return unique[i] < unique[j]
	})

	oldNew := make(map[uint32]int)

	var tiles []image.Image
	tc := tm.TileSet.TileCount
	tcol := tm.TileSet.Columns
	th := tm.TileSet.TileHeight
	tw := tm.TileSet.TileWidth

	for r := 0; r < (tc / tcol); r++ {
		for c := 0; c < tcol; c++ {
			tiles = append(tiles,
				misc.GetSubImage(img, image.Point{c * tw, r * th}, image.Rectangle{
					Min: image.Point{},
					Max: image.Point{th, tw},
				}),
			)
		}
	}

	for index := range tiles {
		oldNew[uint32(index+1)] = indexOfUint32(uint32(index+1), unique)
	}
	remap := make([]uint32, len(layer))

	for i, id := range layer {
		remap[i] = uint32(oldNew[id])
	}

	var uniqueTiles []image.Image
	for i, tile := range tiles {
		if oldNew[uint32(i+1)] != -1 {
			uniqueTiles = append(uniqueTiles, tile)
		}
	}

	oimg := image.NewPaletted(
		image.Rectangle{Min: image.Point{X: 0, Y: 0}, Max: image.Point{X: tw, Y: th * len(uniqueTiles)}},
		img.Palette,
	)

	for i, tile := range uniqueTiles {
		draw.Draw(oimg, image.Rectangle{
			Min: image.Point{0, i * th},
			Max: image.Point{tw, (i + 1) * th},
		}, tile, image.Point{}, draw.Src)
	}

	misc.SavePNG(outName+"_map.png", oimg)

	funcMap := template.FuncMap{
		"endLine": func(i int) bool { return i%tm.Layer.Width == 0 },
	}
	t, err := template.New("export").Funcs(funcMap).Parse(cTileMapTemplate)
	if err != nil {
		return
	}

	ctm := cTileMap{
		name,
		tm.TileSet.TileWidth,
		tm.TileSet.TileHeight,
		tm.TileSet.TileCount,
		tm.Layer.Width,
		tm.Layer.Height,
		remap}

	file, err := os.Create(ctm.Name + "_map.c")
	if err != nil {
		return
	}
	defer file.Close()

	return t.Execute(file, ctm)
}

var printHelp bool
var outName string

func init() {
	flag.BoolVar(&printHelp, "help", false, "print help message and exit\n")
	flag.StringVar(&outName, "name", "", "set optional output name")
}

func main() {
	flag.Parse()

	if len(flag.Args()) < 1 || printHelp {
		flag.PrintDefaults()
		os.Exit(1)
	}

	if len(outName) == 0 {
		outName = misc.PathWithoutExt(flag.Arg(0))
	}

	tm, err := tmx.ReadFile(flag.Arg(0))
	if err != nil {
		log.Fatal(err)
	}

	err = exportTiledMap(tm, outName)
	if err != nil {
		log.Fatal(err)
	}
}
