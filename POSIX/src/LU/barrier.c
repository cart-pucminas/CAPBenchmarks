/*
 * Copyright(C) 2019 Guilherme Reis Barbosa de Oliveira <grboliveira@sga.pucminas.br>
 * 
 * lu.c - Lower Upper Benchmark Kerkernl.
 */


#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#include <fcntl.h>
#include <string.h>

#include "barrier.h"


	

void init_barrier(barrier_t *barrier, int n){
	barrier -> n = n;
	barrier -> count = 0;
	sem_init(&barrier -> mutex, 1,1);
	sem_init(&barrier->turnstyle, 1, 0);
  	sem_init(&barrier->turnstyle2, 1, 0);	
	
}

void close_sem(barrier_t *barrier){
	if(sem_destroy(&barrier -> mutex) == -1){
		perror("Failed destroying barrier");
	}	
	if(sem_destroy(&barrier -> turnstyle) == -1){
		perror("Failed destroying barrier");
	}	
	if(sem_destroy(&barrier -> turnstyle2) == -1){
		perror("Failed destroying barrier");
	}	
}	

void phase1_barrier(barrier_t *barrier){
  sem_wait(&barrier->mutex);
  if (++barrier->count == barrier->n) {
    int i;
    for (i = 0; i < barrier->n; i++) {
      	sem_post(&barrier->turnstyle);
    }
  	}
  	sem_post(&barrier->mutex);
  	sem_wait(&barrier->turnstyle);
}

void phase2_barrier(barrier_t *barrier){
  sem_wait(&barrier->mutex);
  if (--barrier->count == 0) {
    int i;
    for (i = 0; i < barrier->n; i++) {
      	sem_post(&barrier->turnstyle2);
    }
  }
  	sem_post(&barrier->mutex);
  	sem_wait(&barrier->turnstyle2);
}

void wait_barrier(barrier_t *barrier){
	phase1_barrier(barrier);
	phase2_barrier(barrier);
}
	
	

