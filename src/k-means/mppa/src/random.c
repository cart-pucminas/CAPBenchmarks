#include "random.h"

static const unsigned int simple_rng_s_mw_default = 521288629;
static const unsigned int simple_rng_s_mz_default = 362436069;

struct rand_state_struct simple_rng_initialize(int seed) {
  struct rand_state_struct ret;
  unsigned int n1, n2;
  n1 = (seed * 104623) % 4294967296;
  ret.w = (n1) ? n1 : simple_rng_s_mw_default;
  n2 = (seed * 48947) % 4294967296;
  ret.z = (n2) ? n2 : simple_rng_s_mz_default;
  return ret;
}


/* SimpleRNG is a simple random number generator based on
   George Marsaglia's MWC (multiply with carry) generator.
   Although it is very simple, it passes Marsaglia's DIEHARD
   series of random number generator tests.
   Written by John D. Cook */

inline unsigned int simple_rng_next(struct rand_state_struct *current_state) {
  /* 0 <= u < 2^32 */
  unsigned int u;
  current_state->z = 36969 * (current_state->z & 65535) + (current_state->z >> 16);
  current_state->w = 18000 * (current_state->w & 65535) + (current_state->w >> 16);
  u = (current_state->z << 16) + current_state->w;
  return u;
}
