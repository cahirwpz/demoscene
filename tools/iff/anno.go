package iff

type ANNO struct {
	anno string
}

func (a ANNO) Name() string {
	return "ANNO"
}

func (a *ANNO) Read(r Reader) {
	buf := make([]byte, r.Size())
	r.Read(buf)
	a.anno = string(buf)
}

func (a ANNO) String() string {
	return a.anno
}

func makeANNO() Chunk {
	return &ANNO{}
}
