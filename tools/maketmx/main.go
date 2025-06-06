package main

import (
	"encoding/xml"
	"flag"
	"fmt"
	"ghostown.pl/misc"
	"ghostown.pl/tmx"
	"hash/fnv"
	"image"
	"image/color"
	"image/draw"
	"log"
	"os"
	"path"
)

const (
	// N - Single tile size
	N = 16
)

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

func dumpTileMap(tileMap [][]int, width, height int) []uint32 {
	bin := make([]uint32, width*height)

	fmt.Print("\nTile map:\n")

	i := 0
	for y := 0; y < height; y++ {
		fmt.Print("|")
		for x := 0; x < width; x++ {
			ti := uint32(tileMap[y][x] + 1)
			fmt.Printf("%3x", ti)
			bin[i] = ti
			i += 1
		}
		fmt.Printf(" |\n")
	}

	return bin
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

	img, ok := misc.LoadPNG(flag.Arg(0)).(*image.Paletted)
	if !ok {
		log.Fatal("Image is not 8-bit CLUT type!")
	}

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

	fmt.Printf("Distinct / total tiles: %d/%d\n",
		len(tilePos), height/N*width/N)

	reportTileCount(tileCount, tilePos)

	name := misc.PathWithoutExt(flag.Arg(0))
	basename := path.Base(name)

	misc.SavePNG(name+"_tiles.png", tiles)

	reportImage := drawReportOnImage(img, tileMap, tileCount, width/N, height/N)
	misc.SavePNG(name+"_report.png", reportImage)

	outputMap := &tmx.TiledMap{
		Version:     "1.0",
		Orientation: "orthogonal",
		RenderOrder: "left-down",
		Width:       width / N,
		Height:      height / N,
		TileWidth:   N,
		TileHeight:  N}
	outputMap.TileSet = tmx.TiledTileSet{
		FirstGid:   1,
		Name:       basename + "_tiles",
		TileWidth:  N,
		TileHeight: N,
		Spacing:    0,
		TileCount:  len(tilePos),
		Columns:    1}
	outputMap.TileSet.Image = tmx.TiledImage{
		Source: basename + "_tiles.png",
		Width:  tiles.Bounds().Max.X,
		Height: tiles.Bounds().Max.Y}
	outputMap.Layer = tmx.TiledLayer{
		Name:   basename + "_map",
		Width:  width / N,
		Height: height / N}
	outputMap.Layer.Data = tmx.NewTiledData(dumpTileMap(tileMap, width/N, height/N))

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
