### Some useful information on Blizzard 1260 @ 50MHz:

 * chip memory bandwidth is around 6.5 MB/s with DMA turned off
 * fast memory bandwidth is around 35 MB/s
 * cache miss takes 23 cycles and issues 4 fetches 8-5-5-5
 * MOVE16 is the only instruction which avoids reading in the cache line it is writing to
