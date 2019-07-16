#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <semaphore.h>

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

int new_proc(int nprocs){
	int id = 0;
	for(int i = 1;i < nprocs;i++){
		pid_t p = fork();
		if(p == -1){
			perror("fork");
			exit(1);
		}
		else if(p == 0){
			id = i;
			break;
		}
	}
	return id;
}

void close_procs(int nprocs,int id){
	if(id > 0){
		exit(3);
	}
	else{
		for(int i = 1;i < nprocs;i++){
			waitpid(-1,NULL,0);
		}
	}
}

void unlink_sem(const char *name_sem, sem_t *sem){
	if(sem_close(sem) == -1){
		perror("Error closing the semaphore\n");
		exit(0);
	}
	if(sem_unlink(name_sem) == -1){
		perror("Error unlinking the semaphore\n");
		exit(0);
	}
}
