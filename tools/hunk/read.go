package hunk

import (
	"encoding/binary"
	"io"
	"os"
)

func readWord(r io.Reader) (x uint16) {
	if binary.Read(r, binary.BigEndian, &x) != nil {
		panic("no data")
	}
	return
}

func readLong(r io.Reader) (x uint32) {
	if binary.Read(r, binary.BigEndian, &x) != nil {
		panic("no data")
	}
	return
}

func readArrayOfLong(r io.Reader) []uint32 {
	n := readLong(r)
	x := make([]uint32, n)
	for i := 0; i < int(n); i++ {
		x[i] = readLong(r)
	}
	return x
}

func readData(r io.Reader, size uint32) []byte {
	x := make([]byte, size)
	n, err := r.Read(x)
	if err != nil || size != uint32(n) {
		panic("no data")
	}
	return x
}

func skipBytes(r io.Reader, howmany int) {
	readData(r, uint32(howmany))
}

func readStringOfSize(r io.Reader, nlongs uint32) string {
	buf := readData(r, nlongs*4)
	for i := 0; i < int(nlongs*4); i++ {
		if buf[i] == 0 {
			return string(buf[:i])
		}
	}
	return string(buf)
}

func readString(r io.Reader) string {
	nlongs := readLong(r)
	if nlongs == 0 {
		return ""
	}
	return readStringOfSize(r, nlongs)
}

func readArrayOfString(r io.Reader) []string {
	x := make([]string, 1)
	for {
		s := readString(r)
		if s == "" {
			break
		}
		x = append(x, s)
	}
	return x
}

func ReadFile(path string) (hunks []Hunk, err error) {
	file, err := os.Open(path)
	defer file.Close()

	if err != nil {
		return
	}

	var hunkId HunkType

	for {
		err = binary.Read(file, binary.BigEndian, &hunkId)
		if err != nil {
			if err == io.EOF {
				break
			}
			return
		}
		var hunk Hunk
		switch hunkId {
		case HUNK_HEADER:
			hunk = readHunkHeader(file)
		case HUNK_UNIT:
			hunk = readHunkUnit(file)
		case HUNK_NAME:
			hunk = readHunkName(file)
		case HUNK_CODE:
			hunk = readHunkCode(file)
		case HUNK_DATA:
			hunk = readHunkData(file)
		case HUNK_BSS:
			hunk = readHunkBss(file)
		case HUNK_RELOC32:
			hunk = readHunkReloc32(file)
		case HUNK_SYMBOL:
			hunk = readHunkSymbol(file)
		case HUNK_DEBUG:
			hunk = readHunkDebug(file)
		case HUNK_EXT:
			hunk = readHunkExt(file)
		case HUNK_END:
			hunk = HunkEnd{}
		default:
			panic(HunkNameMap[hunkId])
		}
		hunks = append(hunks, hunk)
	}
	return hunks, nil
}
