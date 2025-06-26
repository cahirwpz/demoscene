package main

import (
	"fmt"
	"math"
	"sort"
)

type Row struct {
	err      float64 // average error
	columns  []int   // current set of columns
	removed  int     // which column was removed from working image
	original int     // the column from original picture that was removed
}

func estimate(S []int, n int, N int) []Row {
	if n == 1 {
		R := make([]Row, 1)
		R[0] = Row{0.0, []int{S[0]}, 1, S[1]}
		return R
	}

	di := float64(N-1) / float64(n-1)
	I := make([]float64, n)
	for i := 0; i < n; i++ {
		I[i] = float64(i) * di
	}

	rows := make([]Row, n+1)

	for i := 0; i <= n; i++ {
		C := make([]int, n)
		sum := 0.0
		for j := 0; j < n; j++ {
			if j >= i {
				C[j] = S[j+1]
			} else {
				C[j] = S[j]
			}
			delta := I[j] - float64(C[j])
			sum += delta * delta
		}
		rows[i] = Row{math.Sqrt(sum) / float64(n), C, i, S[i]}
	}

	sort.Slice(rows, func(i, j int) bool {
		return rows[i].err < rows[j].err
	})

	R := estimate(rows[0].columns, n-1, N)
	for _, row := range rows[1:1] {
		C := estimate(row.columns, n-1, N)
		if R[len(R)-1].err > C[len(C)-1].err {
			R = C
		}
	}

	return append(R, rows[0])
}

func main() {
	N := 40
	S := make([]int, N)
	for i := 0; i < N; i++ {
		S[i] = i
	}
	R := estimate(S, N-1, N)
	R = append(R, Row{0.0, S, -1, -1})
	err := 0.0
	for i, row := range R {
		fmt.Printf("%3d: [%d] = %d\n", i+1, row.removed, row.original)
		err += row.err
	}
	fmt.Printf("\naverage error: %.3f\n", err/float64(N-1))
}
