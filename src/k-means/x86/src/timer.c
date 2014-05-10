#include <stdlib.h>
#include <sys/time.h>
#include "timer.h"

static uint64_t residual_error = 0;

void init_time(void) {
  uint64_t t1, t2;
  t1 = get_time();
  t2 = get_time();
  residual_error = t2 - t1;
}

inline uint64_t get_time(void) {
  struct timeval t1;
  uint64_t ret;

  gettimeofday(&t1, NULL);
  ret = UINT64_C(1000000) * ((uint64_t) t1.tv_sec);
  ret += (uint64_t) t1.tv_usec;
  return ret;
}

inline uint64_t diff_time(uint64_t t1, uint64_t t2) {
	return t2 - t1 - residual_error;
}
