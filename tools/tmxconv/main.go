package main

import (
	"flag"
	"image"
	"log"
	"os"
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

func uniqueTileNumbers(layer []uint32, unique []uint32) int {
	/* mark used tile numbers with one */
	for _, num := range layer {
		unique[num] = 1
	}

	/* all used tiles are enumerated starting from 1 */
	var uniqueCount uint32 = 1
	for i, used := range unique {
		if used > 0 {
			unique[i] = uniqueCount
			uniqueCount++
		}
	}

	return int(uniqueCount - 1)
}

func copyTile(ts tmx.TiledTileSet, srcImg *image.Paletted, srcIdx int,
	dstImg *image.Paletted, dstIdx int) {
	tw := ts.TileWidth
	th := ts.TileHeight

	sx := (srcIdx % ts.Columns) * (tw + ts.Spacing)
	sy := (srcIdx / ts.Columns) * (th + ts.Spacing)
	dx := 0
	dy := dstIdx * th

	for y := 0; y < th; y++ {
		for x := 0; x < tw; x++ {
			px := srcImg.ColorIndexAt(sx+x, sy+y)
			dstImg.SetColorIndex(dx+x, dy+y, px)
		}
	}
}

func exportTiledMap(tm tmx.TiledMap, name string) (err error) {
	layer, err := tm.Layer.Data.Decode()
	if err != nil {
		return
	}

	img := misc.LoadPNG(tm.TileSet.Image.Source).(*image.Paletted)

	th := tm.TileSet.TileHeight
	tw := tm.TileSet.TileWidth

	unique := make([]uint32, tm.TileSet.TileCount)
	uniqueCount := uniqueTileNumbers(layer, unique)

	/* optimized layer tile numbers are enumerated starting from 0 */
	optimized := make([]uint32, len(layer))
	for i, id := range layer {
		optimized[i] = unique[id] - 1
	}

	uniqueTileMap := image.NewPaletted(
		image.Rect(0, 0, tw, th*uniqueCount), img.Palette)
	for i, id := range unique {
		if id > 0 {
			copyTile(tm.TileSet, img, i-1, uniqueTileMap, int(id)-1)
		}
	}

	misc.SavePNG(outName+"_map.png", uniqueTileMap)

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
		optimized}

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
