package main

import (
	"fmt"
	"ghostown.pl/hunk"
)

type Reloc struct {
	HunkRef uint32
	Offsets []uint32
}

type SymbolDef struct {
	Type  hunk.ExtType
	Value uint32
}

type SymbolRef struct {
	Type    hunk.ExtType
	Offsets []uint32
}

type Section struct {
	Name string
	Defs map[string]SymbolDef
	Refs map[string]SymbolRef
	Rels []Reloc
}

type Unit struct {
	Name     string
	Sections []Section
}

type LinkJob struct {
	Units []Unit
	Index map[string]Unit
}

func makeSymbolIndex(job LinkJob) map[string]Unit {
	defs := make(map[string]Unit)

	for _, unit := range job.Units {
		for _, sec := range unit.Sections {
			for name, _ := range sec.Defs {
				origUnit, ok := defs[name]
				if ok {
					panic(fmt.Sprintf(
						"%s: redefined symbol in '%s', defined previously in '%s'!",
						name, unit.Name, origUnit.Name))
				}
				defs[name] = unit
			}
		}
	}

	return defs
}

func checkSymbolRefs(job LinkJob) {
	for _, unit := range job.Units {
		for _, sec := range unit.Sections {
			for name, _ := range sec.Refs {
				_, ok := job.Index[name]
				if !ok {
					panic(fmt.Sprintf(
						"%s: not defined symbol in '%s'!", name, unit.Name))
				}
			}
		}
	}
}

func makeLinkJob(hunks []hunk.Hunk) LinkJob {
	var units []Unit
	var unit Unit
	var sect Section

	for i, h := range hunks {
		if h.Type() == hunk.HUNK_UNIT {
			hunkUnit := h.(hunk.HunkStr)
			// Only construct new unit when you hit the next one
			if i > 0 {
				// Remember: append makes a copy of the element!
				units = append(units, unit)
			}
			unit = Unit{Name: hunkUnit.Name, Sections: make([]Section, 0)}
		} else if h.Type() == hunk.HUNK_NAME {
			hunkName := h.(hunk.HunkStr)
			sect = Section{Name: hunkName.Name,
				Defs: make(map[string]SymbolDef),
				Refs: make(map[string]SymbolRef),
				Rels: make([]Reloc, 0)}
		} else if h.Type() == hunk.HUNK_END {
			unit.Sections = append(unit.Sections, sect)
		} else if h.Type() == hunk.HUNK_EXT {
			hunkExt := h.(hunk.HunkExt)
			for _, ext := range hunkExt.Ext {
				if hunk.IsSymDef(ext.Type) {
					_, ok := sect.Defs[ext.Name]
					if ok {
						panic(fmt.Sprintf("%s: symbol %s redefined", unit.Name, ext.Name))
					}
					sect.Defs[ext.Name] = SymbolDef{Type: ext.Type, Value: ext.Value}
				} else if hunk.IsSymRef(ext.Type) {
					_, ok := sect.Refs[ext.Name]
					if ok {
						panic(fmt.Sprintf("%s: another set of references for symbol %s",
							unit.Name, ext.Name))
					}
					sect.Refs[ext.Name] = SymbolRef{Type: ext.Type, Offsets: ext.Refs}
				} else {
					panic("unknown external type")
				}
			}
		} else if h.Type() == hunk.HUNK_SYMBOL {
			hunkSymbol := h.(hunk.HunkSymbol)
			for _, sym := range hunkSymbol.Symbol {
				sect.Defs[sym.Name] = SymbolDef{
					Type: hunk.EXT_DEF, Value: sym.Value}
			}
		} else if h.Type() == hunk.HUNK_RELOC32 {
			hunkReloc := h.(hunk.HunkReloc32)
			for _, rel := range hunkReloc.Reloc {
				sect.Rels = append(sect.Rels,
					Reloc{HunkRef: rel.HunkRef, Offsets: rel.Offsets})
			}
		} else {
			// fmt.Printf("%v not handled!\n", h.Type())
		}
	}

	// Don't forget the last one!
	units = append(units, unit)

	job := LinkJob{Units: units}
	job.Index = makeSymbolIndex(job)
	return job
}
