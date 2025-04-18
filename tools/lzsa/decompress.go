package lzsa

import (
	"log"
)

type Format int

const (
	V1 Format = 0
	V2 Format = 1
)

type decompressor struct {
	input     []uint8
	output    []uint8
	lastIndex int

	nibble    int
	hasNibble bool
}

func (d *decompressor) getByte() int {
	b := int(d.input[d.lastIndex])
	d.lastIndex++
	return b
}

func (d *decompressor) getWord() int {
	w := d.getByte()
	w |= d.getByte() << 8
	return w
}

func (d *decompressor) getByteArray(n int) []uint8 {
	bs := d.input[d.lastIndex : d.lastIndex+n]
	d.lastIndex += n
	return bs
}

func (d *decompressor) lzsa1_block(untilIndex int) {
	var literal int
	var offset int
	var match int

	for {
		/* token: <O|LLL|MMMM> */
		token := d.getByte()

		/* determine literal length */
		literal = (token >> 4) & 7
		if literal == 7 {
			literal += d.getByte()
			if literal == 256 {
				/* a second and third byte follow */
				literal = d.getWord()
			} else if literal > 256 {
				/* a second byte follows */
				literal = 256 + d.getByte()
			}
		}

		/* copy literal */
		d.output = append(d.output, d.getByteArray(literal)...)

		/* end of block in a stream */
		if d.lastIndex == untilIndex {
			return
		}

		/* match offset low */
		if token&0x80 != 0 {
			offset = d.getWord()
			offset |= -65536
		} else {
			offset = d.getByte()
			offset |= -256
		}

		/* optional extra encoded match length */
		match = 3 + (token & 15)
		if match == 3+15 {
			match += d.getByte()
			if match == 256 {
				/* a second and third byte follow */
				match = d.getWord()
				/* end of block */
				if match == 0 {
					return
				}
			} else if match > 256 {
				/* a second byte follows */
				match = 256 + d.getByte()
			}
		}

		/* copy match */
		start := offset + len(d.output)
		for i := 0; i < match; i++ {
			d.output = append(d.output, d.output[start+i])
		}
	}
}

func (d *decompressor) getNibble() int {
	if d.hasNibble {
		d.hasNibble = false
		return d.nibble
	}

	nibble := d.getByte()
	d.hasNibble = true
	d.nibble = nibble & 15
	return nibble >> 4
}

func (d *decompressor) lzsa2_block(untilIndex int) {
	var literal int
	var offset int = -1
	var match int

	for {
		/* token: <XYZ|LL|MMM> */
		token := d.getByte()

		/* determine literal length */
		literal = (token >> 3) & 3
		if literal == 3 {
			literal += d.getNibble()
			if literal == 3+15 {
				literal += d.getByte()
				if literal >= 256 {
					literal = d.getWord()
				}
			}
		}

		/* copy literal */
		d.output = append(d.output, d.getByteArray(literal)...)

		/* end of block in a stream */
		if d.lastIndex == untilIndex {
			return
		}

		/* determine offset */
		code := token >> 5
		invb := (code & 1) ^ 1

		switch code {
		case 0, 1:
			// 00Z: 5-bit offset: read a nibble for offset bits 1-4 and use the inverted bit Z
			// of the token as bit 0 of the offset. set bits 5-15 of the offset to 1.
			offset = d.getNibble() << 1
			offset |= invb
			offset |= -32

		case 2, 3:
			// 01Z 9-bit offset: read a byte for offset bits 0-7 and use the inverted bit Z
			// for bit 8 of the offset. set bits 9-15 of the offset to 1.
			offset = d.getByte()
			offset |= invb << 8
			offset |= -512

		case 4, 5:
			// 10Z 13-bit offset: read a nibble for offset bits 9-12 and use the inverted bit Z
			// for bit 8 of the offset, then read a byte for offset bits 0-7. set bits 13-15 of
			// the offset to 1. substract 512 from the offset to get the final value.
			offset = d.getNibble() << 9
			offset |= invb << 8
			offset |= d.getByte()
			offset |= -8192
			offset -= 512

		case 6:
			// 110 16-bit offset: read a byte for offset bits 8-15, then another byte
			// for offset bits 0-7.
			offset = d.getByte() << 8
			offset |= d.getByte()
			offset |= -65536

		case 7:
			// 111 repeat offset: reuse the offset value of the previous match command.
		}

		/* optional extra encoded match length */
		match = 2 + (token & 7)
		if match == 2+7 {
			match += d.getNibble()
			if match == 2+7+15 {
				match += d.getByte()
				/* end of block ? */
				if match == 256 {
					return
				}
				if match > 256 {
					match = d.getWord()
				}
			}
		}

		/* copy match */
		start := offset + len(d.output)
		for i := 0; i < match; i++ {
			d.output = append(d.output, d.output[start+i])
		}
	}
}

func Decompress(input []uint8) []uint8 {
	d := &decompressor{
		input: input, output: make([]uint8, 0), lastIndex: 0, nibble: 0, hasNibble: false}

	if d.getByte() != 0x7b || d.getByte() != 0x9e {
		log.Fatal("Invalid header")
	}

	var format Format

	if d.getByte()&0x20 != 0 {
		format = V2
	} else {
		format = V1
	}

	for {
		compressed := true

		lo := d.getByte()
		mid := d.getByte()
		hi := d.getByte()

		if lo == 0 && mid == 0 && hi == 0 {
			break
		}

		if hi&0x80 != 0 {
			compressed = false
		}

		length := (hi&1)<<16 | mid<<8 | lo

		if compressed {
			if format == V1 {
				d.lzsa1_block(length + d.lastIndex)
			} else {
				d.lzsa2_block(length + d.lastIndex)
			}
		} else {
			d.output = append(d.output, d.getByteArray(length)...)
		}

		d.nibble = 0
		d.hasNibble = false
	}

	return d.output
}
