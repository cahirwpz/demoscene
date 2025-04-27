package palette

import (
	_ "embed"
	"fmt"
	"image"
	"log"
	"slices"

	"ghostown.pl/png2c/util"
)

//go:embed template.tpl
var tpl string

func Make(in *image.Paletted, cfg image.Config, opts map[string]any) string {
	o := bindParams(opts)

	if !o.StoreUnused {
		// Clean up the palette
		o.Count = int(slices.Max(in.Pix)) + 1
	}

	pal := in.Palette[0:o.Count]

	if len(pal) > o.Count {
		log.Panicf("Expected max %v colors, got %v", o.Count, len(pal))
	}

	// Calculate the color data
	for _, v := range pal {
		if o.Aga {
			c := util.RGB24(v)
			o.ColorsData = append(o.ColorsData, fmt.Sprintf("0x%06x", c))
		} else {
			c := util.RGB12(v)
			o.ColorsData = append(o.ColorsData, fmt.Sprintf("0x%03x", c))
		}
	}

	// Compile the template
	out := util.CompileTemplate(tpl, o)

	return out
}

func bindParams(p map[string]any) (out Opts) {
	out.Name = p["name"].(string)
	out.Count = p["count"].(int)

	if v, ok := p["shared"]; ok {
		out.Shared = v.(bool)
	}
	if v, ok := p["store_unused"]; ok {
		out.StoreUnused = v.(bool)
	}
	if v, ok := p["aga"]; ok {
		out.Aga = v.(bool)
	}

	return out
}

type Opts struct {
	Name        string
	Count       int
	Shared      bool
	StoreUnused bool
	Aga         bool
	// Template-specific data.
	ColorsData []string
}
