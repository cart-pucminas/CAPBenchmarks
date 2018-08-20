#ifndef MASTER_H_
#define MASTER_H_

extern void async_init();
extern void async_start();
extern void spawn_slaves();
extern void join_slaves();

int friendly_numbers(int start_num, int end_num);

#endif