#ifndef __TIMER_H
#define __TIMER_H

#include <inttypes.h>

#define MPPA_FREQUENCY 400

void mppa_init_time(void);
inline uint64_t mppa_get_time(void);
inline uint64_t mppa_diff_time(uint64_t t1, uint64_t t2);

#endif
