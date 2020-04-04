package tmx

import (
	"os"
	"text/template"
)

type TiledMapExport struct {
	Name           string
	TileWidth      int
	TileHeight     int
	TilesCount     int
	LayerWidth     int
	LayerHeight    int
	LayerData      []int
	TemplateString string
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

func (tm *TiledMap) GetTiledMapExportData(name string) (
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
		data,
		tiledMapExportTemplate}
	return
}

func (tme *TiledMapExport) SaveLayerData() (err error) {

	t, err := template.New("export").Parse(tme.TemplateString)
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
