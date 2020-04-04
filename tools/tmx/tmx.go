package tmx

import (
	"bytes"
	"compress/gzip"
	"encoding/base64"
	"encoding/binary"
	"encoding/xml"
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

func (td *TiledData) Decode() (output []uint32, err error) {
	data := strings.TrimSpace(td.Bytes)

	decoded, err := base64.StdEncoding.DecodeString(data)
	if err != nil {
		return
	}

	var gzrd io.Reader
	gzrd, err = gzip.NewReader(bytes.NewBuffer(decoded))
	if err != nil {
		return
	}

	for {
		var word uint32
		err = binary.Read(gzrd, binary.LittleEndian, &word)
		if err == io.EOF {
			err = nil
			break
		}
		if err != nil {
			return
		}
		output = append(output, word)
	}

	return
}

func NewTiledData(data []uint32) TiledData {
	var output bytes.Buffer

	gzwr := gzip.NewWriter(&output)
	for _, word := range data {
		err := binary.Write(gzwr, binary.LittleEndian, word)
		if err != nil {
			log.Panic(err)
		}
	}
	gzwr.Close()

	encoded := base64.StdEncoding.EncodeToString(output.Bytes())

	return TiledData{
		Encoding:    "base64",
		Compression: "gzip",
		Bytes:       encoded}
}

func ReadFile(path string) (tm TiledMap, err error) {
	file, err := os.Open(path)
	defer file.Close()

	if err != nil {
		return
	}

	bytes, err := ioutil.ReadAll(file)
	if err != nil {
		return
	}

	xml.Unmarshal(bytes, &tm)
	return
}
