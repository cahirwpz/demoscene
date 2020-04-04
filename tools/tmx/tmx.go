package tmx

import (
	"bytes"
	"compress/gzip"
	"encoding/base64"
	"encoding/xml"
	"html/template"
	"image"

	"../misc"

	"io"
	"io/ioutil"
	"os"
	"strings"
)

type TiledData struct {
	XMLName     xml.Name `xml:"data"`
	Encoding    string   `xml:"encoding,attr"`
	Compression string   `xml:"compression,attr"`
	Bytes       string   `xml:",chardata"`
}

type TiledLayer struct {
	XMLName xml.Name `xml:"layer"`
	Name    string   `xml:"name,attr"`
	Width   int      `xml:"width,attr"`
	Height  int      `xml:"height,attr"`
	Data    TiledData
}

type TiledImage struct {
	XMLName xml.Name `xml:"image"`
	Source  string   `xml:"source,attr"`
	Width   int      `xml:"width,attr"`
	Height  int      `xml:"height,attr"`
}

// TiledTileSet structure
type TiledTileSet struct {
	XMLName    xml.Name `xml:"tileset"`
	FirstGid   int      `xml:"firstgid,attr"`
	Name       string   `xml:"name,attr"`
	TileWidth  int      `xml:"tilewidth,attr"`
	TileHeight int      `xml:"tileheight,attr"`
	Spacing    int      `xml:"spacing,attr"`
	TileCount  int      `xml:"tilecount,attr"`
	Columns    int      `xml:"columns,attr"`
	Image      TiledImage
}

// TiledMap strucutre
type TiledMap struct {
	XMLName      xml.Name `xml:"map"`
	Version      string   `xml:"version,attr"`
	Orientation  string   `xml:"orientation,attr"`
	RenderOrder  string   `xml:"renderorder,attr"`
	Width        int      `xml:"width,attr"`
	Height       int      `xml:"height,attr"`
	TileWidth    int      `xml:"tilewidth,attr"`
	TileHeight   int      `xml:"tileheight,attr"`
	NextObjectID int      `xml:"nextobjectid,attr,omitempty"`
	TileSet      TiledTileSet
	Layer        TiledLayer
}

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

func (ti *TiledImage) ReadSource() (img *image.Paletted) {
	return misc.LoadPNG(ti.Source)
}

func (tm *TiledMap) GetTiledMapExportData(name string) (
	tmExport TiledMapExport, err error) {

	b, err := tm.Layer.Data.Decompress()
	if err != nil {
		return
	}

	var layerData = make([]int, len(b)/4)

	for i, byte := range b {
		if i%4 == 0 {
			layerData[i/4] = int(byte) - 1
		}
	}
	return TiledMapExport{name, tm.TileSet.TileWidth, tm.TileSet.TileHeight,
		tm.TileSet.TileCount, tm.Layer.Width, tm.Layer.Height, layerData,
		tiledMapExportTemplate}, nil
}

// Decode base64 string and ungzip it to bytes.
func (td *TiledData) Decompress() (out []byte, err error) {
	data := strings.TrimSpace(td.Bytes)

	decoded, err := base64.StdEncoding.DecodeString(data)
	if err != nil {
		return
	}
	b := bytes.NewBuffer(decoded)

	var r io.Reader
	r, err = gzip.NewReader(b)
	if err != nil {
		return
	}

	var buf bytes.Buffer
	_, err = buf.ReadFrom(r)
	if err != nil {
		return
	}

	out = buf.Bytes()
	return
}

// Compress bytes with gzip and convert output to base64 string.
func CompressBytes(data []byte) (out string, err error) {
	var buf bytes.Buffer

	zw := gzip.NewWriter(&buf)
	defer zw.Close()
	_, err = zw.Write(data)
	if err != nil {
		return
	}

	out = base64.StdEncoding.EncodeToString(buf.Bytes())
	return
}

func ReadFile(path string) (parsedMap TiledMap, err error) {
	file, err := os.Open(path)
	defer file.Close()

	if err != nil {
		return
	}

	bytes, err := ioutil.ReadAll(file)
	if err != nil {
		return
	}

	xml.Unmarshal(bytes, &parsedMap)
	return
}

/*
SaveLayerData saves tiles data decompressed using
TiledData Decompress() method as a c file.
*/
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
