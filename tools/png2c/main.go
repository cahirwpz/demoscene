package main

import (
	"flag"
	"fmt"
	"image"
	"io"
	"log"
	"os"
	"strings"

	b "ghostown.pl/png2c/bitmap"
	p "ghostown.pl/png2c/palette"
	pms "ghostown.pl/png2c/params"
	"ghostown.pl/png2c/pixmap"
	"ghostown.pl/png2c/sprite"
	"ghostown.pl/png2c/util"
)

type arrayFlag []string

func (i *arrayFlag) String() string {
	return "my string representation"
}

func (i *arrayFlag) Set(value string) error {
	*i = append(*i, value)
	return nil
}

var (
	bitmapVar  arrayFlag
	pixmapVar  arrayFlag
	spriteVar  arrayFlag
	paletteVar arrayFlag
)

func init() {
	flag.Var(&bitmapVar, "bitmap", "Output Amiga bitmap [name,dimensions,flags]")
	flag.Var(&pixmapVar, "pixmap", "Output pixel map [name,width,height,type,flags]")
	flag.Var(&spriteVar, "sprite", "Output Amiga sprite [name]")
	flag.Var(&paletteVar, "palette", "Output Amiga palette [name,colors]")

	flag.Parse()
}

func main() {
	r, err := os.Open(flag.Arg(0))
	if err != nil {
		log.Panicf("Failed to open file %q", flag.Arg(0))
	}
	file, _ := io.ReadAll(r)

	img, cfg, err := util.DecodePNG(file)
	if err != nil {
		log.Panic(err)
	}

	var out string

	if len(paletteVar) > 0 {
		// Check if image has a palette
		pm, _ := img.(*image.Paletted)
		if pm == nil {
			log.Panic("Only paletted images are supported!")
		}
		for _, flag := range paletteVar {
			opts := pms.ParseOpts(flag,
				pms.Param{Name: "name", CastType: pms.TYPE_STRING},
				pms.Param{Name: "count", CastType: pms.TYPE_INT},
				pms.Param{Name: "shared", CastType: pms.TYPE_BOOL, Value: false},
				pms.Param{Name: "store_unused", CastType: pms.TYPE_BOOL, Value: false},
			)
			out += p.Make(pm, cfg, opts)
		}
	}

	if len(bitmapVar) > 0 {
		// Check if image has a palette
		pm, _ := img.(*image.Paletted)
		if pm == nil {
			log.Panic("only paletted images are supported")
		}

		for _, flag := range bitmapVar {
			opts := pms.ParseOpts(flag,
				pms.Param{Name: "name", CastType: pms.TYPE_STRING},
				pms.Param{Name: "width,height,depth", CastType: pms.TYPE_INT},
				pms.Param{Name: "extract_at", CastType: pms.TYPE_INT, Value: "-1x-1"},
				pms.Param{Name: "interleaved", CastType: pms.TYPE_BOOL, Value: false},
				pms.Param{Name: "cpuonly", CastType: pms.TYPE_BOOL, Value: false},
				pms.Param{Name: "shared", CastType: pms.TYPE_BOOL, Value: false},
				pms.Param{Name: "limit_depth", CastType: pms.TYPE_BOOL, Value: false},
				pms.Param{Name: "onlydata", CastType: pms.TYPE_BOOL, Value: false},
			)

			out += b.Make(pm, cfg, opts)
		}
	}

	if len(pixmapVar) > 0 {
		for _, flag := range pixmapVar {
			opts := pms.ParseOpts(flag,
				pms.Param{Name: "name", CastType: pms.TYPE_STRING},
				pms.Param{Name: "width,height,bpp", CastType: pms.TYPE_INT},
				pms.Param{Name: "limit_bpp", CastType: pms.TYPE_BOOL, Value: false},
				pms.Param{Name: "displayable", CastType: pms.TYPE_BOOL, Value: false},
				pms.Param{Name: "onlydata", CastType: pms.TYPE_BOOL, Value: false},
			)
			out += pixmap.Make(img, cfg, opts)
		}
	}

	if len(spriteVar) > 0 {
		pm, _ := img.(*image.Paletted)
		if pm == nil {
			log.Panic("only paletted images are supported")
		}
		for _, flag := range spriteVar {
			opts := pms.ParseOpts(flag,
				pms.Param{Name: "name", CastType: pms.TYPE_STRING},
				pms.Param{Name: "height", CastType: pms.TYPE_INT},
				pms.Param{Name: "count", CastType: pms.TYPE_INT},
				pms.Param{Name: "attached", CastType: pms.TYPE_BOOL, Value: false},
			)
			out += sprite.Make(pm, cfg, opts)
		}
	}

	inName := strings.Split(r.Name(), ".")[0]
	outName := fmt.Sprintf("%s.c", inName)
	err = os.WriteFile(outName, []byte(out), 0777)
	if err != nil {
		log.Panicf("Failed to write file %q", flag.Arg(0))
	}
}
