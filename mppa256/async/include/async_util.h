#ifndef ASYNC_UTIL_H_
#define ASYNC_UTIL_H_

#ifdef _MASTER_
extern void async_master_start();
extern void async_master_finalize();
#else
/* don't do anything on slave side
   before calling this function    */
extern void async_slave_init();
extern void async_slave_finalize();
#endif

#endif