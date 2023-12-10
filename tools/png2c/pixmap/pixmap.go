package pixmap

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

func Make(in image.Image, cfg image.Config, opts map[string]any) string {
	o := bindParams(opts)

	// Validate bpp
	if o.Bpp != 4 && o.Bpp != 8 && o.Bpp != 12 {
		log.Panicf("Wrong specification: bits per pixel: %v!", o.Bpp)
	}

	// Validate image size
	if o.Width != cfg.Width || o.Height != cfg.Height {
		log.Panicf("Image size is wrong: expected %vx%v, got %vx%v!",
			o.Width, o.Height, cfg.Width, cfg.Height)
	}

	var data []uint
	var dataFmtStr string

	// Handle RGB images
	if rgbm, _ := in.(*image.RGBA); rgbm != nil {
		if o.Bpp <= 8 {
			log.Panic("Expected RGB true color image!")
		}

		// Calculate the data
		o.Size = o.Width * o.Height
		o.Type = "PM_RGB12"
		o.Stride = o.Width
		o.PixType = "u_short"

		// Binary data
		data = rgb12(*rgbm, o.Height, o.Width)
		dataFmtStr = "0x%04x"
	} else {
		// Handle paletted and grayscale images
		var pix []uint8
		if pm, _ := in.(*image.Paletted); pm != nil {
			pix = pm.Pix
		} else if gray, _ := in.(*image.Gray); gray != nil {
			pix = gray.Pix
		} else {
			log.Panic("Expected color mapped or grayscale image!")
		}

		// Set and validate bpp
		if o.Bpp > 8 {
			log.Panic("Depth too big!")
		}
		bpp := util.GetDepth(pix)
		if o.LimitBpp {
			bpp = min(o.Bpp, bpp)
		}

		// Validate image depth
		if o.Bpp < bpp {
			log.Panicf("Image depth is wrong: expected %v, got %v", o.Bpp, bpp)
		}

		// Calculate the data
		if o.Bpp == 4 {
			o.Type = "PM_CMAP4"
			o.Stride = (o.Width + 1) / 2
			data = chunky4(in, pix, o.Width, o.Height)
		} else {
			o.Type = "PM_CMAP8"
			o.Stride = o.Width
			data = make([]uint, 0, len(pix))
			for _, v := range pix {
				data = append(data, uint(v))
			}
		}

		o.Size = o.Stride * o.Height
		o.PixType = "u_char"

		dataFmtStr = "0x%02x"
	}

	for i := 0; i < o.Stride*o.Height; i += o.Stride {
		row := []string{}
		for _, v := range data[i : i+o.Stride] {
			o := fmt.Sprintf(dataFmtStr, v)
			row = append(row, o)
		}
		o.PixData = append(o.PixData, strings.Join(row, ", "))
	}

	out := util.CompileTemplate(tpl, o)

	return out
}

func chunky4(im image.Image, pix []uint8, width, height int) (out []uint) {
	for y := 0; y < height; y++ {
		for x := 0; x < ((width + 1) & ^1); x += 2 {
			var x0, x1 uint8
			if gray, _ := im.(*image.Gray); gray != nil {
				x0 = gray.GrayAt(x, y).Y & 15
				x1 = gray.GrayAt(x+1, y).Y & 15
			} else if pm, _ := im.(*image.Paletted); pm != nil {
				x0 = pm.ColorIndexAt(x, y) & 15
				x1 = pm.ColorIndexAt(x+1, y) & 15
			}
			if x+1 >= width {
				x1 = 0
			}
			out = append(out, uint((x0<<4)|x1))
		}
	}
	return out
}

func rgb12(img image.RGBA, height, width int) (out []uint) {
	for y := 0; y < height; y++ {
		for x := 0; x < width; x++ {
			c := util.RGB12(img.At(x, y))
			out = append(out, c)
		}
	}
	return out
}

func bindParams(p map[string]any) (out Opts) {
	out.Name = p["name"].(string)
	out.Width = p["width"].(int)
	out.Height = p["height"].(int)
	out.Bpp = p["bpp"].(int)

	if v, ok := p["limit_bpp"]; ok {
		out.LimitBpp = v.(bool)
	}
	if v, ok := p["onlydata"]; ok {
		out.OnlyData = v.(bool)
	}
	if v, ok := p["displayable"]; ok {
		out.Displayable = v.(bool)
	}

	return out
}

type Opts struct {
	Name        string
	Width       int
	Height      int
	Bpp         int
	LimitBpp    bool
	Displayable bool
	OnlyData    bool
	// Template-specific data
	Size    int
	Stride  int
	Type    string
	PixData []string
	PixType string
}
