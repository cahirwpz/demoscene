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

func exportTiledMap(tm tmx.TiledMap, name string) (err error) {

	funcMap := template.FuncMap{
		"endLine": func(i int) bool { return i%tm.Layer.Width == 0 },
	}

	t, err := template.New("export").Funcs(funcMap).Parse(cTileMapTemplate)
	if err != nil {
		return
	}

	data, err := tm.Layer.Data.Decode()
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
		data}

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

	img := misc.LoadPNG(tm.TileSet.Image.Source).(*image.Paletted)

	// TODO Add tiles reordering and optimize.

	misc.SavePNG(outName+"_map.png", img)
}
