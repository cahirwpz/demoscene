package main

import (
	"bytes"
	"compress/gzip"
	"encoding/base64"
	"encoding/binary"
	"encoding/xml"
	"flag"
	"fmt"
	"hash/fnv"
	"image"
	"image/color"
	"image/draw"
	"image/png"
	"log"
	"os"
	"path"
)

const (
	// N - Single tile size
	N = 16
)

type tiledData struct {
	XMLName     xml.Name `xml:"data"`
	Encoding    string   `xml:"encoding,attr"`
	Compression string   `xml:"compression,attr"`
	Bytes       string   `xml:",chardata"`
}
type tiledLayer struct {
	XMLName xml.Name `xml:"layer"`
	Name    string   `xml:"name,attr"`
	Width   int      `xml:"width,attr"`
	Height  int      `xml:"height,attr"`
	Data    tiledData
}
type tiledImage struct {
	XMLName xml.Name `xml:"image"`
	Source  string   `xml:"source,attr"`
	Width   int      `xml:"width,attr"`
	Height  int      `xml:"height,attr"`
}
type tiledTileSet struct {
	XMLName    xml.Name `xml:"tileset"`
	FirstGid   int      `xml:"firstgid,attr"`
	Name       string   `xml:"name,attr"`
	TileWidth  int      `xml:"tilewidth,attr"`
	TileHeight int      `xml:"tileheight,attr"`
	Spacing    int      `xml:"spacing,attr"`
	TileCount  int      `xml:"tilecount,attr"`
	Columns    int      `xml:"columns,attr"`
	Image      tiledImage
}
type tiledMap struct {
	XMLName      xml.Name `xml:"map"`
	Version      string   `xml:"version,attr"`
	Orientation  string   `xml:"orientation,attr"`
	RenderOrder  string   `xml:"renderorder,attr"`
	Width        int      `xml:"width,attr"`
	Height       int      `xml:"height,attr"`
	TileWidth    int      `xml:"tilewidth,attr"`
	TileHeight   int      `xml:"tileheight,attr"`
	NextObjectID int      `xml:"nextobjectid,attr,omitempty"`
	TileSet      tiledTileSet
	Layer        tiledLayer
}

func loadPNG(name string) *image.Paletted {
	file, err := os.Open(name)
	defer file.Close()
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

func hashTile(img *image.Paletted, x, y int) uint64 {
	pixels := make([]uint8, N*N)
	for ty := 0; ty < N; ty++ {
		for tx := 0; tx < N; tx++ {
			pixels[ty*N+tx] = img.ColorIndexAt(x+tx, y+ty)
		}
	}

	h := fnv.New64()
	h.Write(pixels)
	return h.Sum64()
}

func drawReportOnImage(baseImg *image.Paletted, tileMap [][]int, tileCount []int, width, height int) image.Image {
	bounds := baseImg.Bounds()
	rgbaImage := image.NewRGBA(bounds)
	redOverlay := image.NewRGBA(bounds)
	red := color.RGBA{255, 0, 0, 255}
	draw.Draw(redOverlay, redOverlay.Bounds(), &image.Uniform{red}, image.ZP, draw.Src)
	rectMask := image.NewRGBA(bounds)
	for y := 0; y < height; y++ {
		for x := 0; x < width; x++ {
			ti := tileMap[y][x]
			if tileCount[ti] <= minTileCount {
				drawRect(rectMask, x*N, y*N, tileCount[ti])
			}
		}
	}
	draw.Draw(rgbaImage, bounds, baseImg, image.ZP, draw.Src)
	draw.DrawMask(rgbaImage, bounds, redOverlay, image.ZP, rectMask, image.ZP, draw.Over)
	return rgbaImage
}

func drawRect(img *image.RGBA, x, y, intensity int) {
	alphaValue := uint8(200 - ((intensity - 1) * 200 / minTileCount))
	square := image.NewRGBA(image.Rect(0, 0, N, N))
	color := color.Alpha{alphaValue}
	draw.Draw(square, square.Bounds(), &image.Uniform{color}, image.ZP, draw.Src)
	draw.Draw(img, image.Rect(x, y, x+N, y+N), square, image.ZP, draw.Over)
}

func makeTiles(tilePos []image.Point, img *image.Paletted) *image.Paletted {
	dim := image.Rect(0, 0, N, len(tilePos)*N)
	tiles := image.NewPaletted(dim, img.Palette)
	for i, src := range tilePos {
		for y := 0; y < N; y++ {
			for x := 0; x < N; x++ {
				px := img.ColorIndexAt(src.X+x, src.Y+y)
				tiles.SetColorIndex(x, y+i*N, px)
			}
		}
	}
	return tiles
}

func pathWithoutExt(p string) string {
	n := len(p) - len(path.Ext(p))
	return p[:n]
}

func compressBytes(data []byte) string {
	var buf bytes.Buffer

	zw := gzip.NewWriter(&buf)
	_, err := zw.Write(data)
	if err != nil {
		log.Fatal(err)
	}
	zw.Close()

	return base64.StdEncoding.EncodeToString(buf.Bytes())
}

func reportTileCount(tileCount []int, tilePos []image.Point) {
	fmt.Printf("\nTiles used less that %d times:\n", minTileCount)

	j := 0
	for i, count := range tileCount {
		if count < minTileCount {
			s := fmt.Sprintf("%v [x%d]", tilePos[i], count)
			fmt.Printf("%20s", s)
			if j%4 == 3 {
				fmt.Print("\n")
			}
			j++
		}
	}

	if j%4 != 3 {
		fmt.Print("\n")
	}
}

func dumpTileMap(tileMap [][]int, width, height int) string {
	bin := make([]byte, width*height*4)

	fmt.Print("\nTile map:\n")

	i := 0
	for y := 0; y < height; y++ {
		fmt.Print("|")
		for x := 0; x < width; x++ {
			ti := tileMap[y][x]
			fmt.Printf("%3x", ti)
			binary.LittleEndian.PutUint32(bin[i:], uint32(ti)+1)
			i += 4
		}
		fmt.Printf(" |\n")
	}

	return compressBytes(bin)
}

func savePNG(name string, img image.Image) {
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
func width(img image.Image) int {
	width := img.Bounds().Dx()
	if width%N != 0 {
		log.Fatalf("Image width must be divisible by %d!", N)
	}
	return width
}
func height(img image.Image) int {
	height := img.Bounds().Dy()
	if height%N != 0 {
		log.Fatalf("Image height must be divisible by %d!", N)
	}
	return height
}

var printHelp bool
var minTileCount int

func init() {
	flag.BoolVar(&printHelp, "help", false, "print help message and exit")
	flag.IntVar(&minTileCount, "min-tile-count", 1,
		"print tile position if it's used less than `N` times")
}

func main() {
	flag.Parse()

	if len(flag.Args()) < 1 || printHelp {
		flag.PrintDefaults()
		os.Exit(1)
	}
	img := loadPNG(flag.Arg(0))
	width := width(img)
	height := height(img)
	tileHash := make(map[uint64]int)
	tileCount := make([]int, 0)
	tileMap := make([][]int, height)
	tilePos := make([]image.Point, 0)
	for y := 0; y < img.Bounds().Dy(); y += N {
		ty := (y - img.Bounds().Min.Y) / N
		tileMap[ty] = make([]int, width)
		for x := 0; x < img.Bounds().Dx(); x += N {
			tx := (x - img.Bounds().Min.X) / N

			th := hashTile(img, x, y)
			ti, ok := tileHash[th]
			if !ok {
				ti = len(tilePos)
				tileHash[th] = ti
				tilePos = append(tilePos, image.Pt(x, y))
				tileCount = append(tileCount, 0)
			}
			tileMap[ty][tx] = ti
			tileCount[ti]++
		}
	}

	tiles := makeTiles(tilePos, img)

	fmt.Printf("Distinct / total tiles: %d/%d\n", len(tilePos), height/N*width/N)

	reportTileCount(tileCount, tilePos)

	name := pathWithoutExt(flag.Arg(0))

	savePNG(name+"_tiles.png", tiles)

	reportImage := drawReportOnImage(img, tileMap, tileCount, width/N, height/N)
	savePNG(name+"_report.png", reportImage)

	outputMap := &tiledMap{
		Version:     "1.0",
		Orientation: "orthogonal",
		RenderOrder: "left-down",
		Width:       width / N,
		Height:      height / N,
		TileWidth:   N,
		TileHeight:  N}
	outputMap.TileSet = tiledTileSet{
		FirstGid:   1,
		Name:       name + "_tiles",
		TileWidth:  N,
		TileHeight: N,
		Spacing:    0,
		TileCount:  len(tilePos),
		Columns:    1}
	outputMap.TileSet.Image = tiledImage{
		Source: name + "_tiles.png",
		Width:  tiles.Bounds().Max.X,
		Height: tiles.Bounds().Max.Y}
	outputMap.Layer = tiledLayer{
		Name:   name + "_map",
		Width:  width / N,
		Height: height / N}
	outputMap.Layer.Data = tiledData{
		Encoding:    "base64",
		Compression: "gzip",
		Bytes:       dumpTileMap(tileMap, width/N, height/N)}

	file, err := os.Create(name + ".tmx")
	if err != nil {
		log.Fatal(err)
	}
	enc := xml.NewEncoder(file)
	enc.Indent("", "  ")
	if err := enc.Encode(outputMap); err != nil {
		fmt.Printf("error: %v\n", err)
	}
	file.Close()
}
