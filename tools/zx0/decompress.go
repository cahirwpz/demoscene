package zx0

type State int

const (
	END                   State = 0
	COPY_LITERALS         State = 1
	COPY_FROM_LAST_OFFSET State = 2
	COPY_FROM_NEW_OFFSET  State = 3
)

const (
	INITIAL_OFFSET = 1
)

type decompressor struct {
	input      []byte
	output     []byte
	lastOffset int
	inputIndex int
	bitMask    byte
	bitValue   byte
	lastByte   byte
}

func (d *decompressor) readByte() byte {
	d.lastByte = d.input[d.inputIndex]
	d.inputIndex++
	return d.lastByte
}

func (d *decompressor) readBit() byte {
	d.bitMask >>= 1
	if d.bitMask == 0 {
		d.bitMask = 128
		d.bitValue = d.readByte()
	}
	if d.bitValue&d.bitMask != 0 {
		return 1
	}
	return 0
}

func (d *decompressor) readGammaCode0() int {
	value := 1
	for d.readBit() == 0 {
		value = (value << 1) | int(d.readBit())
	}
	return value
}

func (d *decompressor) readGammaCode1() int {
	value := 1
	for d.readBit() == 0 {
		value = (value << 1) | int(d.readBit()^1)
	}
	return value
}

func (d *decompressor) readGammaCode2() int {
	value := 1
	value = (value << 1) | int(d.readBit())
	for d.readBit() == 0 {
		value = (value << 1) | int(d.readBit())
	}
	return value
}

func (d *decompressor) copyLiteral(length int) {
	for i := 0; i < length; i++ {
		d.output = append(d.output, d.readByte())
	}
}

func (d *decompressor) copyBytes(length int) {
	for i := 0; i < length; i++ {
		d.output = append(d.output, d.output[len(d.output)-d.lastOffset])
	}
}

func (d *decompressor) decompress() []byte {
	state := COPY_LITERALS

	for {
		switch state {
		case COPY_LITERALS:
			length := d.readGammaCode0()
			d.copyLiteral(length)
			if d.readBit() == 0 {
				state = COPY_FROM_LAST_OFFSET
			} else {
				state = COPY_FROM_NEW_OFFSET
			}
		case COPY_FROM_LAST_OFFSET:
			length := d.readGammaCode0()
			d.copyBytes(length)
			if d.readBit() == 0 {
				state = COPY_LITERALS
			} else {
				state = COPY_FROM_NEW_OFFSET
			}
		case COPY_FROM_NEW_OFFSET:
			msb := d.readGammaCode1()
			if msb == 256 {
				return d.output
			}
			lsb := d.readByte()
			d.lastOffset = int(msb)*128 - int(lsb>>1)
			var length int
			if lsb&1 == 0 {
				length = d.readGammaCode2() + 1
			} else {
				length = 2
			}
			d.copyBytes(length)
			if d.readBit() == 0 {
				state = COPY_LITERALS
			} else {
				state = COPY_FROM_NEW_OFFSET
			}
		}
	}
}

func Decompress(data []byte) []byte {
	d := &decompressor{input: data, lastOffset: INITIAL_OFFSET}
	return d.decompress()
}
