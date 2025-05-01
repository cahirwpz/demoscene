package obj

import (
	"cmp"
	"log"
	"math"
	"slices"
)

/*
 * For given triangle T with vertices A, B and C, surface normal N is a cross
 * product between vectors AB and BC.
 *
 * Ordering of vertices in polygon description is meaningful - depending on
 * that the normal vector will be directed inwards or outwards.
 *
 * Clockwise convention is used.
 */
func CalculateFaceNormal(obj *WavefrontObj, i int) ObjVector {
	face := obj.Faces[i]

	if len(face.Indices) < 3 {
		log.Panicf("cannot compute normal to point/line #%d", i)
	}

	p1 := obj.Vertices[face.Indices[0].Vertex-1]
	p2 := obj.Vertices[face.Indices[1].Vertex-1]
	p3 := obj.Vertices[face.Indices[2].Vertex-1]

	ax := p1[0] - p2[0]
	ay := p1[1] - p2[1]
	az := p1[2] - p2[2]
	bx := p2[0] - p3[0]
	by := p2[1] - p3[1]
	bz := p2[2] - p3[2]

	x := ay*bz - by*az
	y := az*bx - bz*ax
	z := ax*by - bx*ay

	l := math.Sqrt(x*x + y*y + z*z)

	if l == 0 {
		log.Panicf("face #%d normal vector has zero length", i)
	}

	// Normal vector has a unit length.
	return ObjVector{x / l, y / l, z / l}
}

type FaceEdge struct {
	Edge Edge
	Face int
}

func EdgeCmp(a, b Edge) int {
	if n := cmp.Compare(a.Point[0], b.Point[0]); n != 0 {
		return n
	}
	return cmp.Compare(a.Point[1], b.Point[1])
}

func CalculateEdges(obj *WavefrontObj) []Edge {
	var fes []FaceEdge

	/* Create all edges. */
	for i, face := range obj.Faces {
		if len(face.Indices) < 3 {
			continue
		}
		for j := 0; j < len(face.Indices); j++ {
			k := j + 1
			if k == len(face.Indices) {
				k = 0
			}
			p0 := face.Indices[j].Vertex - 1
			p1 := face.Indices[k].Vertex - 1
			if p1 < p0 {
				p0, p1 = p1, p0
			}
			fes = append(fes, FaceEdge{Edge: Edge{Point: [2]int{p0, p1}}, Face: i})
		}
	}

	/* Sort the edges lexicographically. */
	slices.SortFunc(fes, func(a, b FaceEdge) int {
		return EdgeCmp(a.Edge, b.Edge)
	})

	/* Modify existing face index list. */
	var es []Edge
	eis := make([][]int, len(obj.Faces))
	n := -1

	for _, fe := range fes {
		l := len(es)
		if l == 0 || EdgeCmp(es[l-1], fe.Edge) != 0 {
			es = append(es, fe.Edge)
			n++
		}
		eis[fe.Face] = append(eis[fe.Face], n)
	}

	for i, ei := range eis {
		for j, e := range ei {
			obj.Faces[i].Indices[j].Edge = e
		}
	}

	return es
}
