package main

import (
	"flag"
	"image"
	"log"
	"os"
	"path/filepath"
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

var printHelp bool
var sourceName string
var tilesName string

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

func exportTiledMap(tl tmx.TiledLayer, ts tmx.TiledTileSet,
	path string, name string) (err error) {

	layer, err := tl.Data.Decode()
	if err != nil {
		return
	}

	img := misc.LoadPNG(filepath.Join(path, ts.Image.Source)).(*image.Paletted)

	th := ts.TileHeight
	tw := ts.TileWidth

	/* orignally tiles are enumarated starting from 1 */
	unique := make([]uint32, ts.TileCount+1)
	uniqueCount := uniqueTileNumbers(layer, unique)

	/* optimized layer tile numbers are enumerated starting from 0 */
	optimized := make([]uint32, len(layer))
	for i, id := range layer {
		optimized[i] = unique[id] - 1
	}

	if len(tilesName) > 0 {
		uniqueTileMap := image.NewPaletted(
			image.Rect(0, 0, tw, th*uniqueCount), img.Palette)
		for i, id := range unique {
			if id > 0 {
				copyTile(ts, img, i-1, uniqueTileMap, int(id)-1)
			}
		}
		misc.SavePNG(tilesName, uniqueTileMap)
	}

	if len(sourceName) > 0 {
		funcMap := template.FuncMap{
			"endLine": func(i int) bool { return i%tl.Width == 0 },
		}
		t, err := template.New("export").Funcs(funcMap).Parse(cTileMapTemplate)
		if err != nil {
			return err
		}

		ctm := cTileMap{name, ts.TileWidth, ts.TileHeight, ts.TileCount,
			tl.Width, tl.Height, optimized}

		file, err := os.Create(sourceName)
		if err != nil {
			return err
		}
		defer file.Close()

		err = t.Execute(file, ctm)
		if err != nil {
			return err
		}
	}
	return
}

func init() {
	flag.BoolVar(&printHelp, "help", false,
		"print help message and exit")
	flag.StringVar(&sourceName, "source", "",
		"Export layer data to given C source file")
	flag.StringVar(&tilesName, "tiles", "",
		"Export tiles to given PNG file")
}

func main() {
	flag.Parse()

	export := len(sourceName) > 0 || len(tilesName) > 0
	if len(flag.Args()) < 1 || !export || printHelp {
		flag.PrintDefaults()
		os.Exit(1)
	}

	path := filepath.Dir(flag.Arg(0))
	name := misc.PathWithoutExt(filepath.Base(flag.Arg(0)))

	tm, err := tmx.ReadFile(flag.Arg(0))
	if err != nil {
		log.Fatal(err)
	}

	err = exportTiledMap(tm.Layer, tm.TileSet, path, name)
	if err != nil {
		log.Fatal(err)
	}
}
