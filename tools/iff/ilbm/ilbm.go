package ilbm

import ".."

func init() {
	// Add recognized chunks to global list
	iff.ParsedChunks["BMHD"] = makeBMHD
	iff.ParsedChunks["CRNG"] = makeCRNG
	iff.ParsedChunks["CMAP"] = makeCMAP

	// Ignore JUNK chunk
	iff.IgnoredChunks["JUNK"] = true
}
