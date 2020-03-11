package tmx

import (
	"bytes"
	"compress/gzip"
	"encoding/base64"
	"encoding/xml"
	"io"
)

// TiledData sructure
type TiledData struct {
	XMLName     xml.Name `xml:"data"`
	Encoding    string   `xml:"encoding,attr"`
	Compression string   `xml:"compression,attr"`
	Bytes       string   `xml:",chardata"`
}

// TiledLayer structure
type TiledLayer struct {
	XMLName xml.Name `xml:"layer"`
	Name    string   `xml:"name,attr"`
	Width   int      `xml:"width,attr"`
	Height  int      `xml:"height,attr"`
	Data    TiledData
}

// TiledImage structure
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

//DecompresString - decompress base64 gziped string to bytes
func DecompresString(data string) (out []byte, err error) {

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

// CompressBytes - compress bytes to base64 string
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
