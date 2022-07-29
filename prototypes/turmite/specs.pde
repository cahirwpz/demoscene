/*
 * Turmite specification table
 * indexed by turmite state and board color
 * encodes: new color, direction change, new color
 *
 * direction change is:
 * +1 = RIGHT
 * -1 = LEFT
 * +2 = UTURN
 *  0 = NO-TURN
 */
final int[][][] SpiralGrowth = { 
  {
    { 1,  0, 1 },
    { 1, -1, 0 },
  }, {
    { 1, +1, 1 },
    { 0,  0, 0 },
  }
};

final int[][][] ChaoticGrowth = {
  {
    { 1, +1, 1 },
    { 1, -1, 1 },
  }, {
    { 1, +1, 1 },
    { 0, +1, 0 },
  }
};

final int[][][] ExpandingFrame = {
  {
    { 1, -1, 0 },
    { 1, +1, 1 },
  }, {
    { 0, +1, 0 },
    { 0, -1, 1 },
  }
};

final int[][][] FibonacciSpiral = {
  {
    {1, -1, 1},
    {1, -1, 1}
  }, {
    {1, +1, 1},
    {0,  0, 0}
  }
};

final int[][][] Snowflake = {
  {
    {1, -1, 1},
    {1, +1, 0}
  }, {
    {1, +2, 1},
    {1, +2, 2}
  }, {
    {},
    {0, +2, 0}
  } 
};
