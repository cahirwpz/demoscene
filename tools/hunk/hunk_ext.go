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
	Ext []Extern
}

func readHunkExt(r io.Reader) HunkExt {
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
		case EXT_DEF, EXT_ABS, EXT_RES:
			value := readLong(r)
			ext = append(ext, Extern{extType, name, value, nil})
		case EXT_REF32, EXT_REF16, EXT_REF8, EXT_DEXT32, EXT_DEXT16, EXT_DEXT8:
			ext = append(ext, Extern{extType, name, 0, readArrayOfLong(r)})
		case EXT_COMMON:
			size := readLong(r)
			ext = append(ext, Extern{extType, name, size, readArrayOfLong(r)})
		default:
			panic("unknown external type")
		}
	}
	sort.Slice(ext, func(i, j int) bool {
		if ext[i].Type == ext[j].Type {
			return ext[i].Name < ext[j].Name
		}
		return ext[i].Type < ext[j].Type
	})
	return HunkExt{ext}
}

func (h HunkExt) Type() HunkType {
	return HUNK_EXT
}

func (h HunkExt) String() string {
	var sb strings.Builder

	sb.WriteString("HUNK_EXT\n")

	prevExtType := EXT_NONE
	for _, ext := range h.Ext {
		if prevExtType != ext.Type {
			fmt.Fprintf(&sb, " %s:\n", HunkExtNameMap[ext.Type])
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
