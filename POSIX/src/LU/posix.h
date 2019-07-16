#ifndef _POSIX_H_
#define _POSIX_H_

	extern void* open_shared_mem(const char *name_object, int range);

	extern void close_shared_mem(const char *name_object);

	extern int new_proc(int nprocs);

	extern void close_procs(int nprocs,int id);

	extern void unlink_sem(const char *name_sem, sem_t *sem);

#endif
