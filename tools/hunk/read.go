package hunk

import (
	"encoding/binary"
	"io"
	"os"
)

func ReadFile(path string) (hunks []Hunk, err error) {
	file, err := os.Open(path)
	defer file.Close()

	if err != nil {
		return
	}

	var hunkId uint32

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
		case HUNK_END:
			hunk = HunkEnd{}
		default:
			panic(HunkName[hunkId])
		}
		hunks = append(hunks, hunk)
	}
	return hunks, nil
}
