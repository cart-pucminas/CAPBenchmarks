/*
 * Copyright(C) 2014 Guilherme R. Barbosa de Oliveira <grboliveira97@gmail.com>
 * 
 * friendly-numbers.c - Friendly numbers kernel with POSIX.
 */

#include <global.h>
#include <omp.h>
#include <stdlib.h>
#include <util.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "fn.h"
/*
 * Opening shared memory regions
 */
void* open_shared_mem(const char *name_object, int range){
	void *addr;
	int shm_fd = shm_open(name_object, O_CREAT | O_RDWR , 0666);
	if(shm_fd == -1){
		fprintf(stderr, "Open failed:%s\n", strerror(errno));
		exit(1);
	}	
	if(ftruncate(shm_fd,range * sizeof(int)) == -1){
		perror("ftruncate() error");
		exit(1);
	}	
	addr = mmap(0,range * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED,shm_fd,0);
	if(addr == (void *)-1){
		fprintf(stderr, "mmap failed : %s\n",strerror(errno));
	        exit(1);
	}
	return addr;	
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

void unlink_mem(const char *name_object){
	if(shm_unlink(name_object) == -1){
		perror("Unlink shared memory error");
		exit(0);
	}
}	

	




/*
 * Computes the Greatest Common Divisor of two numbers.
 */
static int gcd(int a, int b)
{
  int c;
  
  /* Compute greatest common divisor. */
  while (a != 0)
  {
     c = a;
     a = b%a;
     b = c;
  }
  
  return (b);
}

/*
 * Some of divisors.
 */
static int sumdiv(int n)
{
	int sum;    /* Sum of divisors. */
	int factor; /* Working factor.  */
	
	sum = 1 + n;
	
	/* Compute sum of divisors. */
	for (factor = 2; factor < n; factor++)
	{
		/* Divisor found. */
		if ((n%factor) == 0)
			sum += factor;
	}
	
	return (sum);
}

/*
 * Computes friendly numbers.
 */
int friendly_numbers(int start, int end) 
{

	const char *name_object1 = "Numerator";
	const char *name_object2 = "Denominator";
	const char *name_object3 = "Tasks";
	const char *name_object4 = "nfriends";
	const char *name_object5 = "count";
	const char *name_object6 = "barrier";
	const char *name_object7 = "critical";	

	int n;        /* Divisor.                    */
	int range;    /* Range of numbers.           */
	int i, j;     /* Loop indexes.               */
	int pid;      /* Process id.                 */
	int nprocs;   /* Number of processes         */

	range = end - start + 1;
	nprocs = nthreads;
	/* Shared variables */
	int *num = (int*)open_shared_mem(name_object1,range);     /* Numerator                  */
	int *den = (int*)open_shared_mem(name_object2,range);	  /* Denominator                */
	int *tasks = (int*)open_shared_mem(name_object3,range);	  /* Tasks                      */	
	int *nfriends = (int*)open_shared_mem(name_object4,1);    /* Number of friendly numbers */
	int *count = (int*)open_shared_mem(name_object5,1);	  /* Count                      */
	(*count) = 0;
	(*nfriends) = 0;

	sem_t* barrier[nprocs];
	sem_t *critical;
	sem_t *mutex;
	
	for(int i = 0;i < nprocs;i++){
		barrier[i] = sem_open(name_object6, O_CREAT, 0644, 0);
	}	
	critical = sem_open(name_object7, O_CREAT, 0644, 1);
	mutex = sem_open("mutex", O_CREAT, 0644, 1);

	
	/* Balance workload. */
	balance(tasks, range, nthreads);	
	/* Forking the process */
	pid = new_proc(nprocs);
	/* Compute abundances. */ 
	for (i = start; i <= end; i++) 
	{	
		j = i - start;

		/* Not my task.*/ 
		if (tasks[j] != pid)
			continue;
			
		num[j] = sumdiv(i);
		den[j] = i;
				
		n = gcd(num[j], den[j]);
		num[j] /= n;
		den[j] /= n;
	}	
	/* Barrier */        	
	sem_wait(mutex);
	(*count)++;
	if(*count == nprocs){
		int j = 0;
		while(j < nprocs){
			sem_post(barrier[j]);
			j++;
		}
		(*count) = 0;	
	}	
	sem_post(mutex);
	sem_wait(barrier[pid]);


	/* Check friendly numbers.*/ 
	for (i = (1 + pid); i < range; i+=nprocs)
	{
		for (j = 0; j < i; j++)
		{
			/* Friends.*/
			if ((num[i] == num[j]) && (den[i] == den[j])){
				sem_wait(critical);
				(*nfriends)++;
				sem_post(critical);
			}	
		}	
	}

	/* Closing the processes */
	close_procs(nprocs,pid);
	/* Unlinking the shared memories */	
	unlink_mem(name_object1);
	unlink_mem(name_object2);
	unlink_mem(name_object3);
	unlink_mem(name_object4);
	unlink_mem(name_object5);
	for(int i = 0;i < nprocs;i++){
		sem_close(barrier[i]);
	}	
	/* Closing semaphores */
	sem_close(mutex);
	sem_close(critical);
	printf("Friendly Numbers:%d\n",*nfriends);
	printf("Finishing FN\n");	
	return (*nfriends);
}
