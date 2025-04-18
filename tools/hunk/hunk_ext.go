package hunk

import (
	"fmt"
	"io"
	"sort"
	"strings"
)

type Extern struct {
	Type  ExtType
	Name  string
	Value uint32
	Refs  []uint32
}

type HunkExt struct {
	Externs []Extern
}

func readHunkExt(r io.Reader) *HunkExt {
	var ext []Extern
	for {
		nlongs := readLong(r)
		if nlongs == 0 {
			break
		}
		length := nlongs & 0xffffff
		extType := ExtType(nlongs >> 24)
		name := readStringOfSize(r, length)
		switch extType {
		case EXT_DEF, EXT_ABS, EXT_RES, EXT_GNU_LOCAL:
			value := readLong(r)
			ext = append(ext, Extern{extType, name, value, nil})
		case EXT_REF32, EXT_REF16, EXT_REF8, EXT_DEXT32, EXT_DEXT16, EXT_DEXT8:
			ext = append(ext, Extern{extType, name, 0, readArrayOfLong(r)})
		case EXT_COMMON:
			size := readLong(r)
			ext = append(ext, Extern{extType, name, size, readArrayOfLong(r)})
		default:
			panic(fmt.Sprintf("unknown external type: %v", extType))
		}
	}
	return &HunkExt{ext}
}

func (h *HunkExt) Sort() {
	sort.Slice(h.Externs, func(i, j int) bool {
		if h.Externs[i].Type == h.Externs[j].Type {
			return h.Externs[i].Name < h.Externs[j].Name
		}
		return h.Externs[i].Type < h.Externs[j].Type
	})
}

func (h HunkExt) Type() HunkType {
	return HUNK_EXT
}

func (h HunkExt) Write(w io.Writer) {
	writeLong(w, uint32(HUNK_EXT))
	for _, ext := range h.Externs {
		eh := stringSize(ext.Name) | uint32(ext.Type)<<24
		writeLong(w, eh)
		writeString(w, ext.Name)
		switch ext.Type {
		case EXT_DEF, EXT_ABS, EXT_RES, EXT_GNU_LOCAL:
			writeLong(w, ext.Value)
		case EXT_REF32, EXT_REF16, EXT_REF8, EXT_DEXT32, EXT_DEXT16, EXT_DEXT8:
			writeArrayOfLong(w, ext.Refs)
		case EXT_COMMON:
			writeLong(w, ext.Value)
			writeArrayOfLong(w, ext.Refs)
		}
	}
	writeLong(w, 0)
}

func (h HunkExt) String() string {
	var sb strings.Builder

	sb.WriteString("HUNK_EXT\n")

	prevExtType := EXT_NONE
	for _, ext := range h.Externs {
		if prevExtType != ext.Type {
			fmt.Fprintf(&sb, " %s:\n", ext.Type.String())
			prevExtType = ext.Type
		}
		fmt.Fprintf(&sb, "  name: %s", ext.Name)
		if ext.Refs == nil {
			fmt.Fprintf(&sb, ", value: 0x%x", ext.Value)
		} else {
			if ext.Type == EXT_COMMON {
				fmt.Fprintf(&sb, ", size: 0x%x", ext.Value)
			} else {
				sb.WriteString("\n   offsets: ")
				for _, ref := range ext.Refs {
					fmt.Fprintf(&sb, "0x%x ", ref)
				}
			}
		}
		sb.WriteString("\n")
	}

	return sb.String()
}
