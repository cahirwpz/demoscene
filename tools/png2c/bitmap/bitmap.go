package bitmap

import (
	_ "embed"
	"fmt"
	"image"
	"log"
	"strings"

	"ghostown.pl/png2c/util"
)

//go:embed template.tpl
var tpl string

func Make(in *image.Paletted, cfg image.Config, opts map[string]any) string {
	o := bindParams(opts)

	// Set and validate depth
	depth := util.GetDepth(in.Pix)
	if o.LimitDepth {
		depth = min(o.Depth, depth)
	}

	// Crop the image if needed (--extract_at option)
	pix := in.Pix
	if o.SubImage {
		pix = util.CutImage(o.StartX, o.StartY, o.Width, o.Height, cfg, in.Pix)
		cfg.Width = o.Width
		cfg.Height = o.Height
	}

	// Validate the image size
	if o.Width != cfg.Width || o.Height != cfg.Height {
		log.Panicf("Image size is wrong: expected %vx%v, got %vx%v",
			o.Width, o.Height, cfg.Width, cfg.Height)
	}

	// Validate image depth
	if o.Depth < depth {
		log.Panicf("Image depth is wrong: expected %v, got %v", o.Depth, depth)
	}

	o.BytesPerRow = ((o.Width + 15) & ^15) / 8
	o.WordsPerRow = o.BytesPerRow / 2
	o.BplSize = o.BytesPerRow * o.Height
	o.Size = o.BplSize * o.Depth

	// Calculate the binary data
	bpl := util.Planar(pix, o.Width, o.Height, depth, o.Interleaved)

	for i := 0; i < depth*o.WordsPerRow*o.Height; i = i + o.WordsPerRow {
		words := []string{}
		for x := 0; x < o.WordsPerRow; x++ {
			f := fmt.Sprintf("0x%04x", bpl[i+x])
			words = append(words, f)
		}
		o.BplData = append(o.BplData, strings.Join(words, ","))
	}

	flags := []string{"BM_STATIC"}
	if o.CpuOnly {
		flags = append(flags, "BM_CPUONLY")
	}
	if o.Interleaved {
		flags = append(flags, "BM_INTERLEAVED")
	}
	o.Flags = strings.Join(flags, "|")

	for i := 0; i < depth; i++ {
		offset := 0
		if o.Interleaved {
			offset = i * o.BytesPerRow
		} else {
			offset = i * o.BplSize
		}
		ptr := fmt.Sprintf("(void *)_%s_bpl + %v", o.Name, offset)
		o.BplPtrs = append(o.BplPtrs, ptr)
	}

	out := util.CompileTemplate(tpl, o)

	return out
}

func bindParams(p map[string]any) (out Opts) {
	out.Name = p["name"].(string)
	out.Width = p["width"].(int)
	out.Height = p["height"].(int)
	out.Depth = p["depth"].(int)

	if v, ok := p["interleaved"]; ok {
		out.Interleaved = v.(bool)
	}
	if v, ok := p["limit_depth"]; ok {
		out.LimitDepth = v.(bool)
	}
	if v, ok := p["cpuonly"]; ok {
		out.CpuOnly = v.(bool)
	}
	if v, ok := p["shared"]; ok {
		out.Shared = v.(bool)
	}
	if v, ok := p["onlydata"]; ok {
		out.OnlyData = v.(bool)
	}
	if v, ok := p["displayable"]; ok {
		out.Displayable = v.(bool)
	}

	if coords, _ := p["extract_at"].([]int); coords[0] >= 0 {
		out.SubImage = true
		out.StartX = coords[0]
		out.StartY = coords[1]
	}

	return out
}

type Opts struct {
	Name        string
	Width       int
	Height      int
	Depth       int
	Interleaved bool
	LimitDepth  bool
	CpuOnly     bool
	Shared      bool
	OnlyData    bool
	Displayable bool
	// Template-specific data
	BytesPerRow int
	WordsPerRow int
	Flags       string
	Planes      string
	BplSize     int
	Size        int
	SubImage    bool
	StartX      int
	StartY      int
	BplData     []string
	BplPtrs     []string
}
