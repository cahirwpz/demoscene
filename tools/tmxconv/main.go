package main

import (
	"flag"
	"image"
	"image/draw"
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

func uniqueTileNumbers(layer []uint32, ntiles int) (unique []uint32) {
	/* mark used tile numbers with one */
	unique = make([]uint32, ntiles)
	for _, num := range layer {
		unique[num] = 1
	}

	/* all used tiles are enumerated starting from 1 */
	var num uint32 = 1
	for i, used := range unique {
		if used > 0 {
			unique[i] = num
			num++
		}
	}

	return
}

func cutTile(ts tmx.TiledTileSet, img *image.Paletted, num int) *image.Paletted {
	row := num / ts.Columns
	col := num % ts.Columns
	tw := ts.TileWidth
	th := ts.TileHeight

	tile := image.NewPaletted(image.Rect(0, 0, tw, th), img.Palette)
	sx := col * (tw + ts.Spacing)
	sy := row * (th + ts.Spacing)
	for y := 0; y < th; y++ {
		for x := 0; x < tw; x++ {
			px := img.ColorIndexAt(sx+x, sy+y)
			tile.SetColorIndex(x, y, px)
		}
	}

	return tile
}

func exportTiledMap(tm tmx.TiledMap, name string) (err error) {
	layer, err := tm.Layer.Data.Decode()
	if err != nil {
		return
	}

	img := misc.LoadPNG(tm.TileSet.Image.Source).(*image.Paletted)

	th := tm.TileSet.TileHeight
	tw := tm.TileSet.TileWidth

	unique := uniqueTileNumbers(layer, tm.TileSet.TileCount)

	/* optimized layer tile numbers are enumerated starting from 0 */
	optimized := make([]uint32, len(layer))
	for i, id := range layer {
		optimized[i] = unique[id] - 1
	}

	var uniqueTiles []image.Image
	for i, id := range unique {
		if id > 0 {
			uniqueTiles = append(uniqueTiles, cutTile(tm.TileSet, img, i))
		}
	}

	tileImg := image.NewPaletted(
		image.Rect(0, 0, tw, th*len(uniqueTiles)), img.Palette)

	for i, tile := range uniqueTiles {
		draw.Draw(tileImg, image.Rect(0, i*th, tw, (i+1)*th), tile,
			image.Point{}, draw.Src)
	}

	misc.SavePNG(outName+"_map.png", tileImg)

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
