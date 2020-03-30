package misc

/* Output is interleaved */
func ChunkyToPlanar(input []uint8, width, height, depth int) []uint16 {
	wordsPerRow := ((width + 15) &^ 15) / 16
	output := make([]uint16, wordsPerRow*height*depth)

	for y := 0; y < height; y++ {
		src := input[y*width : (y+1)*width]
		dst := output[y*wordsPerRow*depth : (y+1)*wordsPerRow*depth]
		for d := 0; d < depth; d++ {
			row := dst[d*wordsPerRow : (d+1)*wordsPerRow]
			for x := 0; x < width; x++ {
				bit := uint16(src[x]>>d) & 1
				pos := 15 - (x & 15)
				row[x/16] |= bit << pos
			}
		}
	}

	return output
}

func Deinterleave(input []uint16, width, height, depth int) []uint16 {
	wordsPerRow := ((width + 15) &^ 15) / 16
	output := make([]uint16, wordsPerRow*height*depth)

	dst := 0
	for d := 0; d < depth; d++ {
		src := d * wordsPerRow
		for y := 0; y < height; y++ {
			copy(output[dst:dst+wordsPerRow], input[src:src+wordsPerRow])
			src += wordsPerRow * depth
			dst += wordsPerRow
		}
	}

	return output
}
