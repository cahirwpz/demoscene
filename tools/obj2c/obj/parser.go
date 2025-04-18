package obj

import (
	"bufio"
	"fmt"
	"os"
	"path/filepath"
	"strconv"
	"strings"
)

type ObjVector []float64

type ObjFaceIndices struct {
	Vertex   int
	TexCoord int
	Normal   int
	Edge     int
}

type ObjFace struct {
	Material int
	Indices  []ObjFaceIndices
}

type ObjFaceGroup struct {
	Name    string
	Indices []int
}

type WavefrontObj struct {
	Name      string
	Vertices  []ObjVector
	TexCoords []ObjVector
	Normals   []ObjVector
	Faces     []ObjFace
	Groups    []ObjFaceGroup
}

type MtlColor struct {
	R, G, B float64
}

type WavefrontMtl struct {
	Name             string
	AmbientColor     MtlColor
	DiffuseColor     MtlColor
	SpecularColor    MtlColor
	SpecularExponent float64
	RefractionIndex  float64
	Illumination     int
}

type WavefrontData struct {
	Objects   []*WavefrontObj
	Materials []*WavefrontMtl
}

/* https://paulbourke.net/dataformats/mtl/ */
func ParseWavefrontMtl(filename string) ([]*WavefrontMtl, error) {
	file, err := os.Open(filename)
	if err != nil {
		return nil, fmt.Errorf("failed to open file %q: %v", filename, err)
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)

	var mtls []*WavefrontMtl
	var mtl *WavefrontMtl

	idx := 0
	lc := 1

	for scanner.Scan() {
		line := strings.Trim(scanner.Text(), " ")
		fields := strings.Split(line, " ")
		cmd := fields[0]
		fields = fields[1:]

		switch cmd {
		case "":
		case "#":
			/* empty line or comment */
		case "newmtl":
			if mtl != nil {
				mtls = append(mtls, mtl)
			}
			mtl = &WavefrontMtl{Name: fields[0]}
			idx++
		case "map_Kd":
		case "Ka":
		case "Ks":
		case "Kd":
		case "Ke":
		case "illum":
		case "d":
		case "Ni":
		case "Ns":
		case "Tf":
		default:
			return nil, fmt.Errorf("unknown command '%s' in line %d", cmd, lc)
		}

		lc++
	}

	/* store last seen material */
	mtls = append(mtls, mtl)

	if err := scanner.Err(); err != nil {
		return nil, fmt.Errorf("error while reading object: %s", err)
	}

	return mtls, nil
}

/* https://paulbourke.net/dataformats/obj/ */
func ParseWavefrontObj(filename string) (*WavefrontData, error) {
	file, err := os.Open(filename)
	if err != nil {
		return nil, fmt.Errorf("failed to open file %q: %v", filename, err)
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)

	data := &WavefrontData{}

	/* parser per object state */
	var obj *WavefrontObj

	grps := make([]int, 0)
	mtl := -1

	/* line count for error reporting */
	lc := 1

	for scanner.Scan() {
		line := strings.Trim(scanner.Text(), " ")
		fields := strings.Split(line, " ")
		cmd := fields[0]
		fields = fields[1:]

		switch cmd {
		case "":
		case "#":
			/* empty line or comment */
		case "v":
			/* Geometric vertex, with (x, y, z, [w]) coordinates
			 * w is optional and defaults to 1.0 */
			v, err := parseVector(fields, 3)
			if err != nil {
				return nil, err
			}
			obj.Vertices = append(obj.Vertices, v)
		case "vt":
			/* Texture coordinates, in (u, [v, w]) coordinates, these will vary between 0 and 1
			 * v, w are optional and default to 0.0 */
			vt, err := parseVector(fields, 1)
			if err != nil {
				return nil, err
			}
			obj.TexCoords = append(obj.TexCoords, vt)
		case "vn":
			/* Vertex normal in (x, y, z) form; normals might not be unit vectors */
			vn, err := parseVector(fields, 3)
			if err != nil {
				return nil, err
			}
			obj.Normals = append(obj.Normals, vn)
		case "p", "l", "f":
			/* Point and line elements will be converted to degenerate face */

			/* Faces are defined using lists of vertex, texture and normal
			 * indices in the format vertex_index/texture_index/normal_index for
			 * which each index starts at 1 and increases corresponding to the
			 * order in which the referenced element was defined. */
			if mtl < 0 {
				return nil, fmt.Errorf("material was not selected in line %d", lc)
			}

			if len(grps) == 0 {
				return nil, fmt.Errorf("no group is selected in line %d", lc)
			}

			f := ObjFace{Material: mtl}

			for _, field := range fields {
				indices := strings.Split(field, "/")
				fi := ObjFaceIndices{Vertex: -1, TexCoord: -1, Normal: -1, Edge: -1}

				for i, index := range indices {
					if index == "" {
						continue
					}

					v, err := strconv.ParseInt(index, 10, 32)
					if err != nil {
						return nil, err
					}

					switch i {
					case 0:
						fi.Vertex = int(v)
					case 1:
						fi.TexCoord = int(v)
					case 2:
						fi.Normal = int(v)
					default:
					}
				}

				f.Indices = append(f.Indices, fi)
			}

			for _, i := range grps {
				obj.Groups[i].Indices = append(obj.Groups[i].Indices, len(obj.Faces))
			}

			obj.Faces = append(obj.Faces, f)
		case "mtllib":
			/* it's undefined if `mtllib` can be specified multiple times,
			 * assume it can be done only once */
			path := filepath.Join(filepath.Dir(file.Name()), fields[0])
			data.Materials, err = ParseWavefrontMtl(path)
			if err != nil {
				return nil, err
			}
		case "s":
			/* Smooth group, ignore */
		case "g":
			grps = make([]int, 0)
			for _, name := range fields {
				idx := findGroupIndex(obj.Groups, name)
				if idx < 0 {
					idx = len(obj.Groups)
					obj.Groups = append(obj.Groups, ObjFaceGroup{Name: name, Indices: []int{}})
				}
				grps = append(grps, idx)
			}
		case "o":
			if obj != nil {
				data.Objects = append(data.Objects, obj)
			}
			mtl = -1
			grps = make([]int, 0)
			obj = &WavefrontObj{Name: fields[0]}
		case "usemtl":
			mtl = findMtlIndex(data.Materials, fields[0])
			if mtl < 0 {
				return nil, fmt.Errorf("unknown material '%s' in line %d", fields[0], lc)
			}
		default:
			return nil, fmt.Errorf("unknown command '%s' in line %d", cmd, lc)
		}

		lc++
	}

	if err := scanner.Err(); err != nil {
		return nil, fmt.Errorf("error while reading object: %v", err)
	}

	/* remember to append last object */
	data.Objects = append(data.Objects, obj)

	return data, nil
}

func parseVector(fields []string, length int) (vec ObjVector, err error) {
	if len(fields) < length {
		return nil, fmt.Errorf("expected vector with len %v, got %v", length, len(vec))
	}
	for _, field := range fields {
		v, err := strconv.ParseFloat(field, 64)
		if err != nil {
			return nil, err
		}
		vec = append(vec, v)
	}
	return vec, nil
}

func findMtlIndex(mtls []*WavefrontMtl, name string) int {
	for i, mtl := range mtls {
		if mtl != nil && mtl.Name == name {
			return i
		}
	}

	return -1
}

func findGroupIndex(grps []ObjFaceGroup, name string) int {
	for i, grp := range grps {
		if grp.Name == name {
			return i
		}
	}

	return -1
}
