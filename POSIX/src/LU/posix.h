#ifndef _POSIX_H_
#define _POSIX_H_

	extern void* open_shared_mem(const char *name_object, int range);

	extern void close_shared_mem(const char *name_object);

#endif
