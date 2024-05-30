package obj

import (
	"bufio"
	"fmt"
	"os"
	"path/filepath"
	"strconv"
	"strings"
)

type Vector []float64

type FaceIndices struct {
	Vertex   int
	TexCoord int
	Normal   int
}

type WavefrontObj struct {
	Name      string
	Vertices  []Vector
	TexCoords []Vector
	Normals   []Vector
	Faces     [][]FaceIndices
	Materials map[string]Material
}

type Color struct {
	R, G, B float64
}

type Material struct {
	Name             string
	AmbientColor     Color
	DiffuseColor     Color
	SpecularColor    Color
	SpecularExponent float64
	RefractionIndex  float64
	Illumination     int
}

/* https://paulbourke.net/dataformats/mtl/ */
func ParseWavefrontMtl(filename string) (map[string]Material, error) {
	file, err := os.Open(filename)
	if err != nil {
		return nil, fmt.Errorf("failed to open file %q: %v", filename, err)
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)

	mtls := make(map[string]Material)
	var mtl *Material

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
				mtls[mtl.Name] = *mtl
			}
			mtl = &Material{Name: fields[0]}
		case "Ka":
		case "Ks":
		case "Kd":
		case "Ke":
		case "illum":
		case "d":
		case "Ni":
		case "Ns":
		default:
			return nil, fmt.Errorf("unknown command '%s' in line %d", cmd, lc)
		}

		lc++
	}

	/* store last seen material */
	mtls[mtl.Name] = *mtl

	if err := scanner.Err(); err != nil {
		return nil, fmt.Errorf("error while reading object: %s", err)
	}

	return mtls, nil
}

/* https://paulbourke.net/dataformats/obj/ */
func ParseWavefrontObj(filename string) (*WavefrontObj, error) {
	file, err := os.Open(filename)
	if err != nil {
		return nil, fmt.Errorf("failed to open file %q: %v", filename, err)
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)

	var name string
	var vs []Vector
	var vts []Vector
	var vns []Vector
	var fs [][]FaceIndices
	var mtls map[string]Material

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
			vs = append(vs, v)
		case "vt":
			/* Texture coordinates, in (u, [v, w]) coordinates, these will vary between 0 and 1
			 * v, w are optional and default to 0.0 */
			vt, err := parseVector(fields, 1)
			if err != nil {
				return nil, err
			}
			vts = append(vts, vt)
		case "vn":
			/* Vertex normal in (x, y, z) form; normals might not be unit vectors */
			vn, err := parseVector(fields, 3)
			if err != nil {
				return nil, err
			}
			vns = append(vns, vn)
		case "f":
			/* Faces are defined using lists of vertex, texture and normal
			 * indices in the format vertex_index/texture_index/normal_index for
			 * which each index starts at 1 and increases corresponding to the
			 * order in which the referenced element was defined. */
			var f []FaceIndices

			for _, field := range fields {
				indices := strings.Split(field, "/")
				fi := FaceIndices{Vertex: -1, TexCoord: -1, Normal: -1}

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

				f = append(f, fi)
			}

			fs = append(fs, f)
		case "mtllib":
			path := filepath.Join(filepath.Dir(file.Name()), fields[0])
			mtls, err = ParseWavefrontMtl(path)
			if err != nil {
				return nil, err
			}
		case "p":
		case "l":
			/* Line element */
		case "g":
		case "o":
			name = fields[0]
		case "usemtl":
			/* not implemented, skipped */
		default:
			return nil, fmt.Errorf("unknown command '%s' in line %d", cmd, lc)
		}

		lc++
	}
	if err := scanner.Err(); err != nil {
		return nil, fmt.Errorf("error while reading object: %v", err)
	}
	obj := &WavefrontObj{
		Name: name, Vertices: vs, TexCoords: vts, Normals: vns, Faces: fs, Materials: mtls}
	return obj, nil
}

func parseVector(fields []string, length int) (vec Vector, err error) {
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
