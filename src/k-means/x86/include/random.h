#ifndef __MY_RAND_H
#define __MY_RAND_H

struct rand_state_struct {
  unsigned int w;
  unsigned int z;
};

#define RAND_STATE_T struct rand_state_struct

struct rand_state_struct simple_rng_initialize(int seed);
inline unsigned int simple_rng_next(struct rand_state_struct *current_state);

#endif 
