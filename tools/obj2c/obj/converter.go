package obj

import (
	_ "embed"
	"sort"
	"strings"
	"text/template"
)

//go:embed template.tpl
var tpl string

const (
	WordSize     = 2
	VertexSize   = 7 * WordSize
	VertexOffset = 1 * WordSize
	TexCoordSize = 2 * WordSize
	EdgeSize     = 3 * WordSize
	IndexSize    = WordSize
)

func Convert(data *WavefrontData, cp ConverterParams) (string, error) {
	/* IMPORTANT! make sure no index is equal to 0 */
	geom := Geometry{
		Name:              cp.Name,
		FaceIndexSize:     1,
		FaceTexCoordIndex: -1,
		FaceEdgeIndex:     -1,
	}

	if cp.Textured {
		geom.FaceTexCoordIndex = geom.FaceIndexSize
		geom.FaceIndexSize += 1
	}

	if cp.Edges {
		geom.FaceEdgeIndex = geom.FaceIndexSize
		geom.FaceIndexSize += 1
	}

	/* dump geometry data from all the objects */
	for _, obj := range data.Objects {
		var vertexIndices []int
		var texCoordIndices []int
		var edgeIndices []int
		var faceIndices []int

		/* vertices */
		for _, v := range obj.Vertices {
			vertexIndices = append(vertexIndices,
				len(geom.Vertices)*VertexSize+VertexOffset)
			x := (v[0] + cp.OffsetX) * cp.Scale
			y := (v[1] + cp.OffsetY) * cp.Scale
			z := (v[2] + cp.OffsetZ) * cp.Scale
			geom.Vertices = append(geom.Vertices,
				Vector{0, int(x * 16), int(y * 16), int(z * 16), 0, 0, 0})
		}

		/* texture coordinates */
		if cp.Textured {
			s := cp.TextureScale
			for _, vt := range obj.TexCoords {
				texCoordIndices = append(texCoordIndices,
					len(geom.TexCoords)*TexCoordSize)
				geom.TexCoords = append(geom.TexCoords,
					Vector{int(vt[0] * s), int(vt[1] * s)})
			}
		}

		/* edges */
		if cp.Edges {
			edges := CalculateEdges(obj)
			for _, e := range edges {
				edgeIndices = append(edgeIndices,
					len(geom.Edges)*EdgeSize)
				/* pre-compute vertex indices */
				e.Point[0] = vertexIndices[e.Point[0]]
				e.Point[1] = vertexIndices[e.Point[1]]
				geom.Edges = append(geom.Edges, e)
			}
		}

		/* faces */
		for i, f := range obj.Faces {
			poly := bool(len(f.Indices) >= 3)
			of := Face{Material: f.Material, Count: len(f.Indices)}
			sz := 2
			if poly {
				fn := CalculateFaceNormal(obj, i)
				of.Normal = Vector{int(fn[0] * 4096), int(fn[1] * 4096), int(fn[2] * 4096)}
				sz += 3
			}
			/* the index is set up as if the normal was always present */
			faceIndices = append(faceIndices,
				(geom.FaceDataCount+sz-5)*IndexSize)
			for _, fi := range f.Indices {
				/* precompute vertex / edge indices */
				of.Indices = append(of.Indices, vertexIndices[fi.Vertex-1])
				if poly {
					if cp.Edges {
						of.Indices = append(of.Indices, edgeIndices[fi.Edge])
					}
					if cp.Textured {
						of.Indices = append(of.Indices, texCoordIndices[fi.TexCoord-1])
					}
				}
			}
			geom.Faces = append(geom.Faces, of)
			geom.FaceDataCount += sz + len(of.Indices)
		}

		/* objects & face/edge/vertex groups */
		object := Object{Name: obj.Name}
		for _, g := range obj.Groups {
			og := Group{Name: g.Name}

			vertexSet := make(map[int]bool)
			edgeSet := make(map[int]bool)

			/* calculate per object face group... */
			for _, gi := range g.Indices {
				og.Face.Indices = append(og.Face.Indices, faceIndices[gi])
				for _, fi := range obj.Faces[gi].Indices {
					vertexSet[fi.Vertex-1] = true
					if fi.Edge >= 0 {
						edgeSet[fi.Edge] = true
					}
				}
			}

			/* TODO what about texcoord group? */

			/* ... and corresponding vertex group */
			for vi := range vertexSet {
				og.Vertex.Indices = append(og.Vertex.Indices, vertexIndices[vi])
			}
			sort.Ints(og.Vertex.Indices)

			/* ... and edge group */
			for ei := range edgeSet {
				og.Edge.Indices = append(og.Edge.Indices, edgeIndices[ei])
			}
			sort.Ints(og.Edge.Indices)

			object.Groups = append(object.Groups, og)
		}

		/* add object */
		geom.Objects = append(geom.Objects, object)
	}

	/* determine subarrays position after merging into single array */
	geom.TexCoordOffset = len(geom.Vertices) * VertexSize
	geom.EdgeOffset = geom.TexCoordOffset + len(geom.TexCoords)*TexCoordSize
	geom.FaceDataOffset = geom.EdgeOffset + len(geom.Edges)*EdgeSize
	geom.ObjectDataOffset = geom.FaceDataOffset + geom.FaceDataCount*IndexSize

	/* relocate texture coordinate & edge indices in faces */
	for i, face := range geom.Faces {
		for j := 0; j < len(face.Indices); j += geom.FaceIndexSize {
			if geom.FaceTexCoordIndex > 0 {
				geom.Faces[i].Indices[j+geom.FaceTexCoordIndex] += geom.TexCoordOffset
			}
			if geom.FaceEdgeIndex > 0 {
				geom.Faces[i].Indices[j+geom.FaceEdgeIndex] += geom.EdgeOffset
			}
		}
	}

	/* relocate face/edge indices in groups */
	for i, object := range geom.Objects {
		for j, group := range object.Groups {
			for k := range group.Edge.Indices {
				geom.Objects[i].Groups[j].Edge.Indices[k] += geom.EdgeOffset
			}
			for k := range group.Face.Indices {
				geom.Objects[i].Groups[j].Face.Indices[k] += geom.FaceDataOffset
			}
		}
	}

	/* relocate vertex groups */
	geom.VertexGroupDataOffset = geom.ObjectDataOffset
	for i, object := range geom.Objects {
		for j, group := range object.Groups {
			if len(group.Vertex.Indices) > 0 {
				geom.Objects[i].Groups[j].Vertex.Offset = geom.ObjectDataOffset
				geom.ObjectDataOffset += (len(group.Vertex.Indices) + 1) * IndexSize
			}
		}
	}
	geom.ObjectDataOffset += IndexSize

	/* relocate edge groups */
	geom.EdgeGroupDataOffset = geom.ObjectDataOffset
	for i, object := range geom.Objects {
		for j, group := range object.Groups {
			if len(group.Edge.Indices) > 0 {
				geom.Objects[i].Groups[j].Edge.Offset = geom.ObjectDataOffset
				geom.ObjectDataOffset += (len(group.Edge.Indices) + 1) * IndexSize
			}
		}
	}
	geom.ObjectDataOffset += IndexSize

	/* relocate face groups */
	geom.FaceGroupDataOffset = geom.ObjectDataOffset
	for i, object := range geom.Objects {
		for j, group := range object.Groups {
			if len(group.Face.Indices) > 0 {
				geom.Objects[i].Groups[j].Face.Offset = geom.ObjectDataOffset
				geom.ObjectDataOffset += (len(group.Face.Indices) + 1) * IndexSize
			}
		}
	}
	geom.ObjectDataOffset += IndexSize

	/* relocate objects */
	for i, object := range geom.Objects {
		geom.Objects[i].Offset = geom.ObjectDataOffset + geom.ObjectDataCount*IndexSize
		geom.ObjectDataCount += 1
		for j := range object.Groups {
			geom.Objects[i].Groups[j].Offset = geom.ObjectDataOffset + geom.ObjectDataCount*IndexSize
			geom.ObjectDataCount += 3
		}
	}

	/* output material indices */
	for i, mtl := range data.Materials {
		geom.Materials = append(geom.Materials, FaceMaterial{Name: mtl.Name, Index: i})
	}

	tmpl, err := template.New("template").Parse(tpl)
	if err != nil {
		return "", err
	}

	var buf strings.Builder
	err = tmpl.Execute(&buf, geom)
	if err != nil {
		return "", err
	}

	return buf.String(), nil
}

type ConverterParams struct {
	Name         string
	Scale        float64
	OffsetX      float64
	OffsetY      float64
	OffsetZ      float64
	TextureScale float64
	Edges        bool
	Textured     bool
}

type Vector []int

type Edge struct {
	Flags int
	Point [2]int
}

type Face struct {
	Normal   Vector
	Material int
	Count    int
	Indices  []int
}

type IndexList struct {
	Offset  int
	Indices []int
}

type Group struct {
	Name   string
	Offset int
	Vertex IndexList
	Edge   IndexList
	Face   IndexList
}

type Object struct {
	Name   string
	Offset int
	Groups []Group
}

type FaceMaterial struct {
	Name  string
	Index int
}

type Geometry struct {
	Name string

	FaceDataCount   int
	ObjectDataCount int

	FaceIndexSize     int
	FaceTexCoordIndex int
	FaceEdgeIndex     int

	TexCoordOffset        int
	EdgeOffset            int
	FaceDataOffset        int
	VertexGroupDataOffset int
	EdgeGroupDataOffset   int
	FaceGroupDataOffset   int
	ObjectDataOffset      int

	Vertices  []Vector
	TexCoords []Vector
	Edges     []Edge
	Faces     []Face
	Objects   []Object
	Materials []FaceMaterial
}
