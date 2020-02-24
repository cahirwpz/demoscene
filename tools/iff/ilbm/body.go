package ilbm

import (
	".."
	"bytes"
	"compress/gzip"
	"fmt"
	"io"
)

type BODY struct {
	Data []byte
}

func (body BODY) Name() string {
	return "BODY"
}

func (body *BODY) Read(r iff.Reader) {
	body.Data = make([]byte, r.Size())
	r.Read(body.Data)
}

func unRLE(data []byte) []byte {
	input := bytes.NewBuffer(data)
	output := bytes.NewBuffer(nil)

	for {
		var n, b byte
		var err error
		if n, err = input.ReadByte(); err == io.EOF {
			break
		}
		if n <= 127 {
			if _, err = io.CopyN(output, input, int64(n)+1); err != nil {
				panic(err)
			}
		} else {
			if b, err = input.ReadByte(); err != nil {
				panic(err)
			}
			for i := 0; i < 257-int(n); i++ {
				output.WriteByte(b)
			}
		}
	}

	return output.Bytes()
}

func Deinterleave(data []byte, width, height, depth int) []byte {
	bytesPerRow := ((width + 15) &^ 15) / 8
	output := bytes.NewBuffer(nil)

	for i := 0; i < depth; i++ {
		s := i * bytesPerRow
		for y := 0; y < height; y++ {
			output.Write(data[s : s+bytesPerRow])
			s += bytesPerRow * depth
		}
	}

	return output.Bytes()
}

func inflate(data []byte) []byte {
	var input io.Reader
	var err error

	if input, err = gzip.NewReader(bytes.NewBuffer(data)); err != nil {
		panic(err)
	}

	output := bytes.NewBuffer(nil)

	if _, err = io.Copy(output, input); err != nil {
		panic(err)
	}

	return output.Bytes()
}

func (body *BODY) Decompress(c BodyComp) {
	switch c {
	case CompRLE:
		body.Data = unRLE(body.Data)
	case CompDeflate:
		body.Data = inflate(body.Data)
	default:
	}
}

func (body BODY) String() string {
	return fmt.Sprintf("{%d bytes}", len(body.Data))
}

func makeBODY() iff.Chunk {
	return &BODY{}
}
