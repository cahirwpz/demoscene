package misc

import (
	"path"
)

func PathWithoutExt(p string) string {
	n := len(p) - len(path.Ext(p))
	return p[:n]
}
