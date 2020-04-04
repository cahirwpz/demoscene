package misc

import (
	"image"
	"math"
	"os"
	"text/template"
)

const (
	bitmapTemplate = `static __data_chip uint16_t _{{ name }}_bpl[] = {
	{{ range .Data }}{{ . }}, {{ end }}
};

BitmapT {{ name }} = {
  .width = {{ .Width }},
  .height = {{ .Height }},
  .depth = {{ .Depth }},
  .bytesPerRow = {{ .BytesPerRow }},
  .bplSize = {{ .BplSize }},
  .flags = 0,
  .compression = 0,
  .palette = NULL,
  .pchgTotal = 0,
  .pchg = NULL,
  .planes = { {{ range bplptr }}
    (void *)_{{ name }}_bpl + {{.}},{{ end }}
  }
};

`
)

type Bitmap struct {
	Width  int
	Height int
	Depth  int
	Data   []uint16
}

func (bm Bitmap) BytesPerRow() int {
	return ((bm.Width + 15) &^ 15) / 8
}

func (bm Bitmap) BplSize() int {
	return bm.BytesPerRow() * bm.Height
}

func imageDepth(img *image.Paletted) int {
	maxValue := 0

	for _, d := range img.Pix {
		if maxValue < int(d) {
			maxValue = int(d)
		}
	}

	return int(math.Ceil(math.Log2(float64(maxValue + 1))))
}

func NewBitmap(img *image.Paletted) *Bitmap {
	width := (img.Bounds().Dx() + 15) &^ 15
	height := img.Bounds().Dy()
	depth := imageDepth(img)

	bm := &Bitmap{
		Width:  width,
		Height: height,
		Depth:  depth,
		Data:   make([]uint16, width*height*depth/16)}

	/* Output is interleaved */
	wordsPerRow := width / 16
	for y := 0; y < height; y++ {
		src := img.Pix[y*width : (y+1)*width]
		dst := bm.Data[y*wordsPerRow*depth : (y+1)*wordsPerRow*depth]
		for d := 0; d < depth; d++ {
			row := dst[d*wordsPerRow : (d+1)*wordsPerRow]
			for x := 0; x < width; x++ {
				bit := uint16(src[x]>>d) & 1
				pos := 15 - (x & 15)
				row[x/16] |= bit << pos
			}
		}
	}

	return bm
}

func (bm *Bitmap) Deinterleave() {
	wordsPerRow := bm.BytesPerRow() / 2
	output := make([]uint16, wordsPerRow*bm.Height*bm.Depth)

	dst := 0
	for d := 0; d < bm.Depth; d++ {
		src := d * wordsPerRow
		for y := 0; y < bm.Height; y++ {
			copy(output[dst:dst+wordsPerRow], bm.Data[src:src+wordsPerRow])
			src += wordsPerRow * bm.Depth
			dst += wordsPerRow
		}
	}

	bm.Data = output
}

func (bm *Bitmap) Export(name string) (err error) {
	funcMap := template.FuncMap{
		"name": func() string { return name },
		"bplptr": func() <-chan int {
			ch := make(chan int)
			go func() {
				defer close(ch)
				for i := 0; i < bm.Depth; i++ {
					ch <- i * bm.BplSize()
				}
			}()
			return ch
		}}

	t, err := template.New(name).Funcs(funcMap).Parse(bitmapTemplate)
	if err != nil {
		return
	}

	return t.Execute(os.Stdout, bm)
}
