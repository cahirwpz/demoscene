package tmx

import (
	"bytes"
	"compress/gzip"
	"encoding/base64"
	"encoding/binary"
	"encoding/xml"
	"fmt"
	"image"
	"image/png"
	"io"
	"io/ioutil"
	"log"
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
	return loadPNG(ti.Source)
}

func (tm *TiledMap) SaveTiledMapInfo(baseName string) (err error) {
	info := fmt.Sprintf("static TileSetT %s = TILESET(%d, %d, %d, \"%s\");\n", tm.TileSet.Name, tm.TileSet.TileWidth, tm.TileSet.TileHeight, tm.TileSet.TileCount, baseName+"_tiles.png")
	info += fmt.Sprintf("static TileMapT %s = TILEMAP(%d, %d, \"%s\");", tm.Layer.Name, tm.Layer.Width, tm.Layer.Height, baseName+"_map.bin")

	file, err := os.Create(baseName + ".h")
	if err != nil {
		return
	}
	defer file.Close()

	err = ioutil.WriteFile(file.Name(), []byte(info), 0644)
	if err != nil {
		return
	}
	return nil
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
SaveLayerData saves tiles data decompressed using TiledData Decompress() method as a binary file.
*/
func SaveLayerData(name string, data []byte) (err error) {

	var tilesCount = make([]byte, len(data)/4)

	for i, byte := range data {
		if i%4 == 0 {
			tilesCount[i/4] = byte - 1
		}
	}

	var sw = make([]byte, len(tilesCount)*2)

	for i, byte := range tilesCount {
		binary.BigEndian.PutUint16(sw[i*2:], uint16(byte))
	}

	file, err := os.Create(name + "_map.bin")
	if err != nil {
		return
	}
	defer file.Close()

	err = ioutil.WriteFile(file.Name(), []byte(sw), 0644)
	if err != nil {
		return
	}
	return nil
}

func SavePNG(name string, img image.Image) {
	f, err := os.Create(name)
	if err != nil {
		log.Fatal(err)
	}

	if err := png.Encode(f, img); err != nil {
		f.Close()
		log.Fatal(err)
	}

	if err := f.Close(); err != nil {
		log.Fatal(err)
	}
}

func loadPNG(name string) *image.Paletted {
	file, err := os.Open(name)
	if err != nil {
		log.Fatal(err)
	}

	someImg, err := png.Decode(file)
	if err != nil {
		log.Fatal(err)
	}
	file.Close()

	img, ok := someImg.(*image.Paletted)
	if !ok {
		log.Fatal("Image is not 8-bit CLUT type!")
	}
	return img
}
