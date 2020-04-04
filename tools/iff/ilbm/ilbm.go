package ilbm

import ".."

func init() {
	// Add recognized chunks to global list
	iff.ParsedChunks["BMHD"] = makeBMHD
	iff.ParsedChunks["CRNG"] = makeCRNG
	iff.ParsedChunks["CMAP"] = makeCMAP
	iff.ParsedChunks["BODY"] = makeBODY
	iff.ParsedChunks["CAMG"] = makeCAMG

	// Ignore JUNK chunk
	iff.IgnoredChunks["JUNK"] = true
}
