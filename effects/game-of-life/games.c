typedef void (BlitterPhaseFunc)(const BitmapT*, const BitmapT*, const BitmapT*, const BitmapT*, u_short minterms);

typedef struct BlitterPhaseT {
  BlitterPhaseFunc* blitfunc;
  u_short minterm;
  u_char srca;
  u_char srcb;
  u_char srcc;
  u_char dst;
} BlitterPhaseT;

typedef struct GameDefinitionT {
  const BlitterPhaseT* phases;
  u_short num_phases;
} GameDefinitionT;

static void BlitAdjacentHorizontal(const BitmapT *sourceA, const BitmapT* sourceB,
                                   const BitmapT *sourceC, const BitmapT *target,
                                   u_short minterms);

static void BlitAdjacentVertical(const BitmapT *sourceA, const BitmapT* sourceB,
                                 const BitmapT *sourceC, const BitmapT *target,
                                 u_short minterms);

static void BlitFunc(const BitmapT *sourceA, const BitmapT* sourceB,
                     const BitmapT *sourceC, const BitmapT *target,
                     u_short minterms);

static void WireworldSwitch(__attribute__((unused)) const BitmapT *sourceA,
                            __attribute__((unused)) const BitmapT *sourceB,
                            __attribute__((unused)) const BitmapT *sourceC,
                            __attribute__((unused)) const BitmapT *target,
                            __attribute__((unused)) u_short minterms);

#define PHASE(sa, sb, sc, d, mt, bf) \
  (BlitterPhaseT){.blitfunc=bf, .minterm=mt, .srca=sa, .srcb=sb, .srcc=sc, .dst=d}
#define PHASE_SIMPLE(s, d, mt, bf) PHASE(s, 0, 0, d, mt, bf)

static const BlitterPhaseT coag_phases[10] = {
  PHASE_SIMPLE(0, 1, FULL_ADDER, BlitAdjacentHorizontal),
  PHASE_SIMPLE(0, 2, FULL_ADDER_CARRY, BlitAdjacentHorizontal),
  PHASE_SIMPLE(1, 3, NABC | ANBC | ABNC | ABC, BlitAdjacentVertical),
  PHASE_SIMPLE(1, 4, NABC | ANBC | ABNC | NANBNC, BlitAdjacentVertical),
  PHASE_SIMPLE(2, 5, NABC | ANBC | ABNC | NANBNC, BlitAdjacentVertical),
  PHASE_SIMPLE(2, 6, NABC | ANBC | ABNC | ABC,  BlitAdjacentVertical),
  PHASE(3, 5, 4, 7, NANBC | NABNC | NABC | ANBNC | ABC, BlitFunc),
  PHASE(4, 6, 3, 8, NANBNC | NANBC | NABNC | ABNC | ABC, BlitFunc),
  PHASE(8, 0, 4, 9, NANBC | NABNC | ABC, BlitFunc),
  PHASE(7, 9, 0, 0, NANBNC | NANBC | NABC | ABC, BlitFunc)
};

static const GameDefinitionT coagulation = {
    .phases = coag_phases,
    .num_phases = 10
};

static const BlitterPhaseT gol_phases[9] = {
  PHASE_SIMPLE(0, 1, FULL_ADDER, BlitAdjacentHorizontal),
  PHASE_SIMPLE(0, 2, FULL_ADDER_CARRY, BlitAdjacentHorizontal),
  PHASE_SIMPLE(1, 3, NANBC | NABNC | NABC | ANBNC | ANBC | ABNC, BlitAdjacentVertical),
  PHASE_SIMPLE(1, 4, NANBNC | NABC | ANBC | ABNC, BlitAdjacentVertical),
  PHASE_SIMPLE(2, 5, NANBNC | NABC | ANBC | ABNC, BlitAdjacentVertical),
  PHASE_SIMPLE(2, 6, NANBNC | ABC, BlitAdjacentVertical),
  PHASE(5, 0, 6, 7, NABNC | ANBNC | ANBC | ABC, BlitFunc),
  PHASE(4, 7, 6, 8, NANBC | NABC | ANBNC | ANBC, BlitFunc),
  PHASE(5, 3, 8, 0, NABNC | ANBC, BlitFunc)
};
  
static const GameDefinitionT classic_gol = {
  .phases = gol_phases,
  .num_phases = 9
};

static const BlitterPhaseT maze_phases[10] = {
  PHASE_SIMPLE(0, 1, FULL_ADDER, BlitAdjacentHorizontal),
  PHASE_SIMPLE(0, 2, FULL_ADDER_CARRY, BlitAdjacentHorizontal),
  PHASE_SIMPLE(1, 3, NANBC | NABNC | ANBNC | ABC, BlitAdjacentVertical),
  PHASE_SIMPLE(1, 4, NABC | ANBC | ABNC | ABC, BlitAdjacentVertical),
  PHASE_SIMPLE(2, 5, NANBNC | ABC, BlitAdjacentVertical),
  PHASE_SIMPLE(2, 6, NABC | ANBC | ABNC | ABC, BlitAdjacentVertical),
  PHASE(4, 6, 5, 7, NANBC | NABNC | ANBNC | ABC, BlitFunc),
  PHASE(3, 5, 7, 8, NANBNC | NANBC | NABNC | ANBC, BlitFunc),
  PHASE(6, 7, 6, 9, NANBC | NABNC | ANBNC | ANBC | ABC, BlitFunc),
  PHASE(9, 0, 8, 0, NANBNC | NABNC | NABC | ABC, BlitFunc),
};

static const GameDefinitionT maze = {
  .phases = maze_phases,
  .num_phases = 10
};

static const BlitterPhaseT diamoeba_phases[9] = {
  PHASE_SIMPLE(0, 1, FULL_ADDER, BlitAdjacentHorizontal),
  PHASE_SIMPLE(0, 2, FULL_ADDER_CARRY, BlitAdjacentHorizontal),
  PHASE_SIMPLE(1, 3, NANBNC | NABC | ANBC | ABNC, BlitAdjacentVertical),
  PHASE_SIMPLE(1, 4, NANBNC | NANBC | NABNC | ANBNC, BlitAdjacentVertical),
  PHASE_SIMPLE(2, 5, NANBNC | NANBC | NABNC | ANBNC, BlitAdjacentVertical),
  PHASE_SIMPLE(2, 6, NANBNC | NABC | ANBC | ABNC, BlitAdjacentVertical),
  PHASE(3, 0, 5, 7, NANBC | NABNC | ANBNC | ABNC, BlitFunc),
  PHASE(6, 4, 7, 8, NABNC | ANBNC | ABNC | ABC, BlitFunc),
  PHASE(5, 7, 8, 0, NANBC | NABNC | ABNC, BlitFunc),
};

static const GameDefinitionT diamoeba = {
  .phases = diamoeba_phases,
  .num_phases = 9
};

static const BlitterPhaseT stains_phases[10] = {
  PHASE_SIMPLE(0, 1, FULL_ADDER, BlitAdjacentHorizontal),
  PHASE_SIMPLE(0, 2, FULL_ADDER_CARRY, BlitAdjacentHorizontal),
  PHASE_SIMPLE(1, 3, NABC | ANBC | ABNC | ABC, BlitAdjacentVertical),
  PHASE_SIMPLE(1, 4, NANBNC | ABC, BlitAdjacentVertical),
  PHASE_SIMPLE(2, 5, NABC | ANBC | ABNC | ABC, BlitAdjacentVertical),
  PHASE_SIMPLE(2, 6, NANBC | NABNC | ANBNC | ABC, BlitAdjacentVertical),
  PHASE(3, 6, 0, 7, NANBC | ABNC, BlitFunc),
  PHASE(7, 5, 4, 8, NANBC | ANBNC | ANBC | ABC, BlitFunc),
  PHASE(8, 3, 5, 9, NANBNC | NANBC | NABNC | ANBNC, BlitFunc),
  PHASE(8, 9, 6, 0, NANBNC | NANBC | NABC | ANBNC, BlitFunc),
};

static const GameDefinitionT stains = {
  .phases = stains_phases,
  .num_phases = 10
};

static const BlitterPhaseT day_and_night_phases[10] = {
  PHASE_SIMPLE(0, 1, FULL_ADDER, BlitAdjacentHorizontal),
  PHASE_SIMPLE(0, 2, FULL_ADDER_CARRY, BlitAdjacentHorizontal),
  PHASE_SIMPLE(1, 3, NANBNC | NANBC | NABNC | ANBNC, BlitAdjacentVertical),
  PHASE_SIMPLE(1, 4, NANBNC | ABC, BlitAdjacentVertical),
  PHASE_SIMPLE(2, 5, NABC | ANBC | ABNC | ABC, BlitAdjacentVertical),
  PHASE_SIMPLE(2, 6, NANBC | NABNC | ANBNC | ABC, BlitAdjacentVertical),
  PHASE(3, 0, 6, 7, NANBNC | NABC | ANBC | ABNC, BlitFunc),
  PHASE(7, 4, 0, 8, NABNC | NABC | ABNC, BlitFunc),
  PHASE(8, 6, 3, 9, NANBNC | NANBC | ANBC | ABNC | ABC, BlitFunc),
  PHASE(5, 9, 7, 0, NANBC | ANBNC | ANBC | ABC, BlitFunc),
};

static const GameDefinitionT day_and_night = {
  .phases = day_and_night_phases,
  .num_phases = 10
};

static const BlitterPhaseT three_four_phases[9] = {
  PHASE_SIMPLE(0, 1, FULL_ADDER, BlitAdjacentHorizontal),
  PHASE_SIMPLE(0, 2, FULL_ADDER_CARRY, BlitAdjacentHorizontal),
  PHASE_SIMPLE(1, 3, NANBNC | NABC | ANBC | ABNC, BlitAdjacentVertical),
  PHASE_SIMPLE(1, 4, NANBNC | NANBC | NABNC | ANBNC, BlitAdjacentVertical),
  PHASE_SIMPLE(2, 5, NANBC | NABNC | ANBNC | NABC | ANBC | ABNC, BlitAdjacentVertical),
  PHASE_SIMPLE(2, 6, NANBC | NABNC | ANBNC | ABC, BlitAdjacentVertical),
  PHASE(3, 0, 6, 7, NANBC | NABNC | ANBNC | ABNC, BlitFunc),
  PHASE(5, 6, 7, 8, NANBNC | ANBC | ABNC | ABC, BlitFunc),
  PHASE(7, 8, 4, 0, NABNC | ABC, BlitFunc),
};

static const GameDefinitionT three_four = {
  .phases = three_four_phases,
  .num_phases = 9
};

// take GOL board for rule B12345678/S12345678, only leave pixels which lie on the
// circuit, and remove pixels which were lit one and two generations ago
static const BlitterPhaseT wireworld_phases1[11] = {
  // 0 = state from one generation ago
  // 1 = state from two generations ago
  // 11 = circuit
  PHASE_SIMPLE(0, 2, FULL_ADDER, BlitAdjacentHorizontal), 
  PHASE_SIMPLE(0, 3, FULL_ADDER_CARRY, BlitAdjacentHorizontal),
  PHASE_SIMPLE(2, 4, NANBNC | NABC | ANBC | ABNC, BlitAdjacentVertical),
  PHASE_SIMPLE(2, 5, NABC | ANBC | ABNC | ABC, BlitAdjacentVertical),
  PHASE_SIMPLE(3, 6, NANBNC | NANBC | NABNC | ANBNC | NABC | ANBC | ABNC, BlitAdjacentVertical),
  PHASE_SIMPLE(3, 7, NANBNC | ABC, BlitAdjacentVertical),
  PHASE(7, 4, 0, 8, ANBC | ABNC | ABC, BlitFunc),
  PHASE(6, 5, 8, 9, NANBNC | NANBC | NABC | ANBNC | ABNC | ABC, BlitFunc),
  PHASE(11, 9, 0, 10, ABNC, BlitFunc),
  PHASE(1, 10, 10, 1, NABC, BlitFunc),
  PHASE(0, 0, 0, 0, 0x0, WireworldSwitch),
  //PHASE(10, 10, 10, 0, ABC, BlitFunc), 
};

static const BlitterPhaseT wireworld_phases2[11] = {
  // 1 = state from one generation ago
  // 0 = state from two generations ago
  // 11 = circuit
  PHASE_SIMPLE(1, 2, FULL_ADDER, BlitAdjacentHorizontal),
  PHASE_SIMPLE(1, 3, FULL_ADDER_CARRY, BlitAdjacentHorizontal),
  PHASE_SIMPLE(2, 4, NANBNC | NABC | ANBC | ABNC, BlitAdjacentVertical),
  PHASE_SIMPLE(2, 5, NABC | ANBC | ABNC | ABC, BlitAdjacentVertical),
  PHASE_SIMPLE(3, 6, NANBNC | NANBC | NABNC | ANBNC | NABC | ANBC | ABNC, BlitAdjacentVertical),
  PHASE_SIMPLE(3, 7, NANBNC | ABC, BlitAdjacentVertical),
  PHASE(7, 4, 1, 8, ANBC | ABNC | ABC, BlitFunc),
  PHASE(6, 5, 8, 9, NANBNC | NANBC | NABC | ANBNC | ABNC | ABC, BlitFunc),
  PHASE(11, 9, 1, 10, ABNC, BlitFunc),
  PHASE(0, 10, 10, 0, NABC, BlitFunc),
  PHASE(0, 0, 0, 0, 0x0, WireworldSwitch),
};

static const GameDefinitionT wireworld1 = {
  .phases = wireworld_phases1,
  .num_phases = 11
};

static const GameDefinitionT wireworld2 = {
  .phases = wireworld_phases2,
  .num_phases = 11
};

static const GameDefinitionT* wireworlds[2] = {
  &wireworld1,
  &wireworld2
};