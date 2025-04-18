package sprite

import (
	_ "embed"
	"fmt"
	"image"
	"log"

	"ghostown.pl/png2c/util"
)

//go:embed template.tpl
var tpl string

func Make(in *image.Paletted, cfg image.Config, opts map[string]any) string {
	o := bindParams(opts)

	// Validate image' size
	if o.Width != cfg.Width || o.Height != cfg.Height {
		got := fmt.Sprintf("%vx%v", cfg.Height, cfg.Width)
		exp := fmt.Sprintf("%vx%v", o.Height, o.Width)
		log.Panicf("Image size is wrong: expected %q, got %q", exp, got)
	}

	// Validate depth
	depth := util.GetDepth(in.Pix)
	if !o.Attached && depth != 2 {
		log.Panicf("Image depth is %v, expected 2!", depth)
	}

	var stride int = ((o.Width + 15) & ^15) / 16
	// Binary data
	bpl := util.Planar(in.Pix, o.Width, o.Height, depth, true)
	n := o.Width / 16
	if o.Attached {
		n = n * 2
	}

	// Calculate the sprite data
	o.Count = n
	o.Sprites = make([]Sprite, n)
	for i := 0; i < n; i++ {
		o.Sprites[i].Name = o.Name
		if o.Width > 16 {
			o.Sprites[i].Name += fmt.Sprintf("%v", i)
		}

		o.Sprites[i].Attached = o.Attached && i%2 == 1

		offset := 0
		if o.Sprites[i].Attached {
			offset = stride * 2
		}
		if o.Attached {
			offset += i / 2
		} else {
			offset += i
		}

		words := []string{}
		for j := 0; j < (stride * depth * o.Height); j += (stride * depth) {
			a := bpl[offset+j]
			b := bpl[offset+j+stride]
			words = append(words, fmt.Sprintf("{ 0x%04x, 0x%04x },", a, b))
		}
		words = append(words, "/* sprite channel terminator */")
		words = append(words, "{ 0x0000, 0x0000 },")

		o.Sprites[i].Data = words
		o.Sprites[i].Height = o.Height
	}

	out := util.CompileTemplate(tpl, o)

	return out
}

func bindParams(p map[string]any) (out Opts) {
	out.Name = p["name"].(string)
	out.Height = p["height"].(int)
	out.Count = p["count"].(int)
	if v, ok := p["attached"]; ok {
		out.Attached = v.(bool)
	}
	out.Width = out.Count * 16

	return out
}

type Opts struct {
	Count int
	Sprite
	// Template-specific data
	Sprites []Sprite
}

type Sprite struct {
	Name     string
	Height   int
	Attached bool
	// Template-specific data
	Data  []string
	Width int
}
