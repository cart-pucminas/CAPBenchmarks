/**************************************
 * TIME
 **************************************/

#include "timer.h"
#include <mppa/osconfig.h>

static uint64_t residual_error = 0;

void mppa_init_time(void) {
  uint64_t t1, t2;
  t1 = mppa_get_time();
  t2 = mppa_get_time();
  residual_error = t2 - t1;
}

inline uint64_t mppa_get_time(void) {
  return __k1_io_read64((void *)0x70084040) / MPPA_FREQUENCY;
}

inline uint64_t mppa_diff_time(uint64_t t1, uint64_t t2) {
  return t2 - t1 - residual_error;
}
