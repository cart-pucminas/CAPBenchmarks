/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * lu.c - Lower Upper Benchmark Kerkernl.
 */

#include <assert.h>
#include <global.h>
#include <math.h>
#include <omp.h>
#include <util.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include "barrier.h"
#include "lu.h"

/*
 * Arguments sanity check.
 */
#define SANITY_CHECK() \
	assert(m != NULL); \
	assert(l != NULL); \
	assert(u != NULL); \

/*
 * Swaps two rows of a matrix.
 */
int *count;
int pid;
int *j0_red;
int nprocs;
int *ipvt;
int *jpvt;
double *pivot;
sem_t *critical;
sem_t *mutex;
barrier_t* barrier;

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
		fprintf(stderr, "CLOSE_PROCS(if): PID:%d\tGetPID:%d\n",id,getpid());
		exit(3);
		fprintf(stderr, "Closed process\n");
	}	
	else{
		fprintf(stderr, "CLOSE_PROCS(else): PID:%d\tGetPID:%d\n",id,getpid());
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




static void _swap_rows(struct matrix *m, int i1, int i2)
{
	int j;     /* Loop index.      */
	double tmp; /* Temporary value. */
	int size;
	size = (m->width)/nprocs;
	//fprintf(stderr, "Before parallel region\n");
	//fprintf(stderr,"Opening %d processes \n",nprocs);
	/*	
	fprintf(stderr, "PID:%d\n",pid);
	fprintf(stderr, "Width:%d\n",m->width);
	fprintf(stderr, "nprocs:%d\n",nprocs);
	fprintf(stderr, "size*pid:%d\n",size*pid);
	fprintf(stderr, "size*(pid+1):%d\n",size*(pid+1));
	*/
	/* Swap columns. */
	wait_barrier(barrier);
	//matrix_show(m);
	for (j = size*pid; j < size*(pid+1); j++)
	{
		//fprintf(stderr, "PID:%d,J:%d\twidth:%d\n",pid,j,m->width);
		tmp = MATRIX(m, i1, j);
		//fprintf(stderr, "PID:%d,tmp:%f\n",pid,tmp);
		wait_barrier(barrier);
		MATRIX(m, i1, j) = MATRIX(m, i2, j);
		//if(pid == 1)fprintf(stderr, "PID:%d,matrix1\n",pid);
		MATRIX(m, i2, j) = tmp;
		//fprintf(stderr, "PID:%d,matrix2\twidth:%d\tnprocs:%d\tj:%d\tsize*(pid+1):%d\tget_omp_thread_num:%d\n",pid,m->width,nprocs,j,size*(pid+1),omp_get_thread_num());

	}
	//fprintf(stderr, "SWAP_ROWS:FOR AREA DID PID:%d\n",pid);
	//fprintf(stderr, "Closed all processes in SWAP ROWS\n");
}

/*
 * Swaps two columns in a matrix.
 */
static void _swap_columns(struct matrix *m, int j1, int j2)
{
	int i;     /* Loop index.      */
	double tmp; /* Temporary value. */
	int size;
	size = (m->height)/nprocs;
	wait_barrier(barrier);
	/* Swap columns. */
	for (i = pid*size; i < size*(pid+1); i++)
	{
		tmp = MATRIX(m, i, j1);
		MATRIX(m, i, j1) = MATRIX(m, i, j2);
		MATRIX(m, i, j2) = tmp;
	}
	//fprintf(stderr, "Closed all processes in SWAP COLUMNS\n");
}

/*
 * Finds the pivot element.
 */
static double _find_pivot(struct matrix *m, int i0, int j0)
{
	int i, j;         /* Loop indexes.          */
	int pipvt, pjpvt; /* Private pivot indexes. */
	int size;         /* Process computation size*/	
	/* Pivot indexes.         */
	size = (m->height)/nprocs;
	//fprintf(stderr,"PID:%d\ti0:%d\tj0:%d\n",pid,i0,j0);	
	
	if(pid == 0){
		(*ipvt) = i0;
		(*jpvt) = j0;
	}
	wait_barrier(barrier);
	//fprintf(stderr,"PID:%d\t*ipvt:%d\t*jpvt:%d\n",pid,*ipvt,*jpvt);
	
	pipvt = i0;
	pjpvt = j0;

	//fprintf(stderr,"PID%d\t*pipvt:%d\tpjpvt:%d\n",pid,pipvt,pjpvt);
	
	//fprintf(stderr,"PID:%d\tsize*pid:%d\t+1:%d\n",pid,pid*size,(pid+1)*size);	
	/* Find pivot element. */
	for (i = size*pid; i < size*(pid+1); i++)
	{
		//fprintf(stderr,"PID:%d\nFIND_PIVOT:i:%d\n",pid,i); 
		for (j = j0; j < m->width; j++)
		{
			/* Found. */
			if (fabs(MATRIX(m, i, j)) < fabs(MATRIX(m,pipvt,pjpvt)))
			{
				pipvt = i;
				pjpvt = j;
			}
		}
		//fprintf(stderr,"PID:%d\tfabs(MATRIX(m,i,j):%f\tfabsMATRIX(m,pipvt,pjpvt):%f\n",pid,fabs(MATRIX(m,i,j)),fabs(MATRIX(m,i,j))); 
	}
	//fprintf(stderr,"PID:%d\tFIND_PIVOT:pipvt:%d\tpjpvt:%d\n",pid,pipvt,pjpvt);
	//fprintf(stderr,"PID:%dFIND_PIVOT:BEFORE SEM\n",pid);
	/* Reduct. */
	//fprintf(stderr,"PID:%dFIND_PIVOT:BEFORE SEM\n",pid);
	if (fabs(MATRIX(m, pipvt, pjpvt) > fabs(MATRIX(m, (*ipvt), (*jpvt) ))))
	{
		sem_wait(critical);
		(*ipvt) = pipvt;
		(*jpvt) = pjpvt;
		sem_post(critical);
		fprintf(stderr,"INSIDE_IF\tPID:%d\t*ipvt:%d\t*jpvt:%d\n",pid,*ipvt,*jpvt);
	}
	wait_barrier(barrier);
	//fprintf(stderr,"PID:%dFIND_PIVOT:AFTER SEM\n",pid);
	//fprintf(stderr, "Passed thorugh parallel area\n");
	_swap_rows(m, i0, *ipvt);
	wait_barrier(barrier);
	//fprintf(stderr, "Passed through swap rows\n");
	_swap_columns(m, j0, *jpvt);
	wait_barrier(barrier);
	//fprintf(stderr, "Passed through all swaps\n");
	//fprintf(stderr,"PID:%d\tFIND_PIVOT:%f\n",pid,MATRIX(m, *ipvt, *jpvt));
	return (MATRIX(m, *ipvt, *jpvt));
}

/*
 * Applies the row reduction algorithm in a matrix.
 */
static void _row_reduction(struct matrix *m, int i0, double pivot)
{
	int i, j;    /* Loop indexes.    */
	double mult;  /* Row multiplier.  */
	int size;	
	size = (m->height)/nprocs;
	if(pid == 0){
		*j0_red = i0;
	}	
	wait_barrier(barrier);
	/* Apply row redution in some lines. */
	for (i = (pid*i0) + 1; i < size*(pid+1); i++)
	{
		mult = MATRIX(m, i, *j0_red)/pivot;
	
		/* Store multiplier. */
		MATRIX(m, i, *j0_red) = mult;
	
		/* Iterate over columns. */
		for (j = *j0_red + 1; j < m->width; j++)
			MATRIX(m, i, j) = MATRIX(m, i, j) - mult*MATRIX(m, i0, j);
	}

	
}

/*
 * Performs LU factorization in a matrix.
 */
int lower_upper(struct matrix *m, struct matrix *l, struct matrix *u)
{
	int i, j;    /* Loop indexes. */
	int size;	/* Size  */
	float lpivot;
	ipvt = (int *)open_shared_mem("ipvt",1 * sizeof(int));
	jpvt = (int *)open_shared_mem("jpvt",1 * sizeof(int));
	j0_red = (int *)open_shared_mem("j0_red",1 * sizeof(int));
	count = (int *)open_shared_mem("count",1 * sizeof(int));
	pivot = (double *)open_shared_mem("pivot",1 * sizeof(double));
  	/* Starting column. */

	size = (m->height)/nprocs;
	nprocs = nthreads;
//	fprintf(stderr, "M->Height:%d\nSize:%d\n",m->height,size);
	/* Shared memory variables */
	int *val = (int *)open_shared_mem("Validation",1 * sizeof(int));
	int *count = (int *)open_shared_mem("Count",1 * sizeof(int));
//	fprintf(stderr, "Initialized all variables\n");
	/* Semaphores */
	barrier = open_shared_mem("barrier",sizeof(barrier_t));
	init_barrier(barrier,nprocs);
	mutex = sem_open("mutex", O_CREAT, 0644, 1);
	critical = sem_open("critical", O_CREAT, 0644, 1);
//	fprintf(stderr, "Set semaphores\n");
	(*count) = 0;
	(*val) = 1;
//	fprintf(stderr, "Count:%d\n",*count);
	pid = new_proc(nprocs);
//	fprintf(stderr, "Created the new processPID:%d\n",pid);	
	/* Apply elimination on all rows. */	
	for (i = 0; i < m->height - 1; i++)
	{
		if(pid==0)fprintf(stderr,"\n");
		lpivot = _find_pivot(m, i, i);
		//fprintf(stderr, "PID:%d\tLPIVOT:%f\n",pid,lpivot);	
		if(pid == 0){
			*pivot = lpivot;
		}	
		wait_barrier(barrier);
		//fprintf(stderr, "PID:%d\tPIVOT:%f\n",pid,*pivot);
		/* Impossible to solve. */
		if (*pivot == 0.0)
		{
			(*val) = 0;
			warning("cannot factorize matrix");
			return (-1);
		}
		//fprintf(stderr ,"Verified pivot\n");	
		if(0)_row_reduction(m, i, *pivot);
		wait_barrier(barrier);
	}
	wait_barrier(barrier);
//	fprintf(stderr, "Passed through elimination area\n");
	if(*val == 0){
		fprintf(stderr, "Aborting...");
		exit(0);
	}
//	fprintf(stderr, "Barrier:PID:%d\n",pid);
//	fprintf(stderr, "Passed through barrier area\n");
//	fprintf(stderr, "PID:%d - SIZE*(PID+1):%d\n",pid,size*(pid+1);
	/* Build upper and lower matrixes.  */
	for (i = (pid*size); i < size*(pid+1); i++)
	{
		for (j = 0; j < m->width; j++)
		{
			if (i > j)
			{
				MATRIX(l, i, j) = 0.0;
				MATRIX(u, i, j) = MATRIX(m, i, j);
			}
			
			else if (i < j)
			{	
				MATRIX(l, i, j) = MATRIX(m, i, j);
				MATRIX(u, i, j) = 0.0;
			}
			
			else
			{
				MATRIX(l, i, j) = 1.0;
				MATRIX(u, i, j) = 1.0;
			}
		}
	}
//	fprintf(stderr, "Exited parallel region\n");
	close_procs(nprocs,pid);
	unlink_mem("Validation");
	unlink_mem("Count");
	close_sem(barrier);
	sem_close(mutex);
	sem_close(critical);	
	return (0);
}
