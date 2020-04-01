package tmx

import (
	"bytes"
	"compress/gzip"
	"encoding/base64"
	"encoding/xml"
	"fmt"
	"image"
	"strconv"

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

func (ti *TiledImage) ReadSource() (img *image.Paletted) {
	return misc.LoadPNG(ti.Source)
}

func (tm *TiledMap) GetTiledMapTemplate(baseName string) (out string) {
	out = fmt.Sprintf("static TileSetT %s_tiles = {\n", baseName)
	out += fmt.Sprintf(".width = %d,\n.height = %d\n.count = %d,\n.ptrs = NULL\n};\n", tm.TileSet.TileWidth, tm.TileSet.TileHeight, tm.TileSet.TileCount)
	out += fmt.Sprintf("const int %s_map_width = %d;\nconst int %s_map_height = %d;\n", baseName, tm.Layer.Width, baseName, tm.Layer.Height)
	out += fmt.Sprintf("short %s_map[] = ", baseName)
	return
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
SaveLayerData saves tiles data decompressed using TiledData Decompress() method as a c file.
*/
func SaveLayerData(name string, data []byte, template string) (err error) {

	var tilesCount = make([]int, len(data)/4)

	for i, byte := range data {
		if i%4 == 0 {
			tilesCount[i/4] = int(byte) - 1
		}
	}

	template += "{"

	for i, el := range tilesCount {
		template += fmt.Sprintf("%s", strconv.Itoa(el))
		if i != len(tilesCount)-1 {
			template += ","
		}
	}

	template += "};"

	file, err := os.Create(name + "_map.c")
	if err != nil {
		return
	}
	defer file.Close()

	err = ioutil.WriteFile(file.Name(), []byte(template), 0644)
	if err != nil {
		return
	}
	return nil
}
