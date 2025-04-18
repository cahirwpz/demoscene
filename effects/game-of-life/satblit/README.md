# Cellular automata to blits tool

This tool can generate blitter configurations to compute next generation of cellular automata from a previous generation.
It generates a series of blitter configurations (passes) that start from a bitmap of a previous generation, run blitter on it and several intermediate results from those blitter phases and produce a new bitmap with the next generation.

This is done by:
1. Generating an input for a SAT-solver
2. Running a SAT-solver on it
3. Converting the result from a SAT-solver to blitter configs (if solution found)

## Prerequisites

- OCaml and the `ocamlbuild` build system
- Any SAT-solver. `minisat` is a go to, but some more powerful ones might be required for more complex rulesets.

## Compiling

```
ocamlbuild satblit.native
```

## Usage

1. Generate input 
```
./satblit.native <N> -o <survive> <born> > solverinput.cnf
```

Where:
- `N` is the amount of blits *in addition to 6 predefined blits* that must be present (2 of those are hardcoded, 4 of those use predefined inputs but blitter function is searched for by the SAT-solver)
- `survive` is a string containing digits 0-8 that specifies how many alive neighbours a cell needs to survive (if it's alive). Default: 23
- `born` is a string containing digits 0-8 that specifies how many alive neighbours a cell needs to become alive (if it's dead). Default: 3

Example for [long life cellular automata](https://conwaylife.com/wiki/OCA:LongLife):
```
./satblit.native 3 -o 5 345 > solverinput.cnf
```

2. Run a SAT-solver
Example using `minisat`:
```
minisat solverinput.cnf solveroutput.sat
```

If the formula was satisfiable `minisat` should print `SATISFIABLE` and produce the output file.
Otherwise if the formula was not satisfiable and `UNSATISFIABLE` was printed then you need to go back to step 1. and increase N.

3. Convert SAT-solver output to blitter passes:

```
./satblit.native <N> -d < solveroutput.sat
```

Value of `N` has to be the same as the one used to generate the input in step 1. corresponding to the output from step 2.
Result is a list of macros of blitter configs that have to be run in sequence in order to calculate the next generation.
They are used in a structure describing a set of blitter passes, see [here](https://github.com/cahirwpz/ghostown-electric-lifeforms/blob/478ca16e16fb5fd6446b255df8066ed984248b18/intro/gol-games.c#L48) for an example.

# Acknowledgements

**Piotr Polesiuk** - for the original idea to use a SAT-solver for finding blitter configurations and for kindly donating the initial version of the code contained here.

