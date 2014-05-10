#ifndef __TIMER_H
#define __TIMER_H

#include <stdint.h>

void init_time(void);
inline uint64_t get_time(void);
inline uint64_t diff_time(uint64_t t1, uint64_t t2);

#endif
