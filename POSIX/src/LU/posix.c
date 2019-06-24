#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#include "posix.h"

void* open_shared_mem(const char *name_object, int range){
	void *addr;
	int shm_fd = shm_open(name_object, O_CREAT | O_RDWR , 0666);
	if(shm_fd == -1){
		fprintf(stderr, "Open failed:%s\n", strerror(errno));
		exit(1);
	}	
	if(ftruncate(shm_fd,range) == -1){
		perror("ftruncate() error");
		exit(1);
	}	
	addr = mmap(0, range, PROT_READ | PROT_WRITE, MAP_SHARED,shm_fd,0);
	if(addr == (void *)-1){
		fprintf(stderr, "mmap failed : %s\n",strerror(errno));
	        exit(1);
	}
	return addr;	
}

void close_shared_mem(const char *name_object){
	if(shm_unlink(name_object)  == -1){
		fprintf(stderr, "Unlink failed:%s\n", strerror(errno));
		exit(1);
	}	
}


