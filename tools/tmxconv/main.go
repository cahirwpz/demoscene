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

type TiledMapExport struct {
	Name        string
	TileWidth   int
	TileHeight  int
	TilesCount  int
	LayerWidth  int
	LayerHeight int
	LayerData   []int
}

var tiledMapExportTemplate = `
static TileSetT {{.Name}}_tiles = {
	.width = {{ .TileWidth}},
	.height = {{ .TileHeight}},
	.count = {{ .TilesCount}},
	.ptrs = NULL
};
const int {{ .Name}}_map_width = {{ .LayerWidth}};
const int {{ .Name}}_map_height = {{ .LayerHeight}};
short {{ .Name}}_map[] = { {{ with .LayerData }}{{ range . }}{{.}},{{ end }}{{ end }} };`

func GetTiledMapExportData(tm tmx.TiledMap, name string) (
	tme TiledMapExport, err error) {

	b, err := tm.Layer.Data.Decompress()
	if err != nil {
		return
	}

	var data = make([]int, len(b)/4)

	for i, byte := range b {
		if i%4 == 0 {
			data[i/4] = int(byte) - 1
		}
	}

	tme = TiledMapExport{
		name,
		tm.TileSet.TileWidth,
		tm.TileSet.TileHeight,
		tm.TileSet.TileCount,
		tm.Layer.Width,
		tm.Layer.Height,
		data}
	return
}

func (tme *TiledMapExport) SaveLayerData() (err error) {

	t, err := template.New("export").Parse(tiledMapExportTemplate)
	if err != nil {
		return
	}

	file, err := os.Create(tme.Name + "_map.c")
	if err != nil {
		return
	}
	defer file.Close()

	err = t.Execute(file, tme)
	if err != nil {
		return
	}
	return nil
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

	var parsedMap tmx.TiledMap
	parsedMap, err := tmx.ReadFile(flag.Arg(0))
	if err != nil {
		log.Fatal(err)
	}

	exportData, err := GetTiledMapExportData(parsedMap, outName)
	if err != nil {
		log.Fatal(err)
	}

	err = exportData.SaveLayerData()
	if err != nil {
		log.Fatal(err)
	}

	img := misc.LoadPNG(parsedMap.TileSet.Image.Source).(*image.Paletted)

	// TODO Add tiles reordering and optimize.

	misc.SavePNG(outName+"_map.png", img)
}
