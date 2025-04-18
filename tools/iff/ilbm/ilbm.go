package ilbm

import "ghostown.pl/iff"

func init() {
	// Add recognized chunks to global list
	iff.ParsedChunks["BMHD"] = makeBMHD
	iff.ParsedChunks["CRNG"] = makeCRNG
	iff.ParsedChunks["CMAP"] = makeCMAP
	iff.ParsedChunks["BODY"] = makeBODY
	iff.ParsedChunks["CAMG"] = makeCAMG
	iff.ParsedChunks["DRNG"] = makeDRNG

	// Ignore JUNK chunk
	iff.IgnoredChunks["JUNK"] = true
}
