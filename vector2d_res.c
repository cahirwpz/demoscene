#include "gfx/common.h"
#include "std/resource_internal.h"

RSC_TYPE(cross, PointT)
RSC_ARRAY(cross) = {
  {1, 0}, {2, 0}, {2, 1}, {3, 1}, {3, 2}, {2, 2}, {2, 3}, {1, 3}, {1, 2}, {0, 2}, {0, 1}, {1, 1}
};

RSC_START
RSC_CONST(cross)
RSC_END
