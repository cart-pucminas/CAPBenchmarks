# 
# File:   master.cpp - Mutually Friendly Numbers master process.
# Authors: 
#		Matheus A. Souza <matheusalcantarasouza@gmail.com>
#		Pedro H. Penna <pedrohenriquepenna@gmail.com>
# 	Copyright(C) 2014
#

# MPPA toolchain.
MPPADIR = /usr/local/k1tools
CC = $(MPPADIR)/bin/k1-gcc

# Toolchain configuration.
export CFLAGS = -Wall -Wextra -O3

# Executable file.
EXEC = mfn

# Define some constants
START=100
END=1000
NUM_PROCS=16
NB_THREADS=16
SHOW_RESULTS=0

# Source files.
SRC_MASTER = master.c
SRC_SLAVE = slave.c

# Builds everything
all: defines master slave
	$(MPPADIR)/bin/createImage.rb --toolchain $(MPPADIR) --clusters=slave --boot=master -T $(EXEC).mpk

defines:
	@echo "#define START $(START)" > defines.h
	@echo "#define END $(END)" >> defines.h
	@echo "#define NUM_PROCS $(NUM_PROCS) " >> defines.h
	@echo "#define SHOW_RESULTS $(SHOW_RESULTS) " >> defines.h
	@echo "#define NB_THREADS $(NB_THREADS) " >> defines.h
	
# Builds mppa master application
master: $(SRC_MASTER)
	$(CC) -mos=rtems $(CFLAGS) $(SRC_MASTER) -o master -lmppaipc 

# Builds mppa master application
slave: $(SRC_SLAVE)
	$(CC) -mos=nodeos $(CFLAGS) $(SRC_SLAVE) -o slave -lmppaipc -fopenmp -D _USE_OMP_ -D NB_THREADS=$(NB_THREADS)

# Cleans compilation files.
clean:
	rm -f master
	rm -f slave
	rm -f $(EXEC).mpk
