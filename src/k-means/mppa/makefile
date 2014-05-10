#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Directories. 
export BINDIR  = $(CURDIR)/bin
export INCDIR  = $(CURDIR)/include
export SRCDIR  = $(CURDIR)/src
export MPPADIR = /usr/local/k1tools

# Executable files.
export EXEC = dna

# Toolchain.
export CC = $(MPPADIR)/bin/k1-gcc

# Toolchain configuration.
export CFLAGS = -I $(INCDIR) -Wall -Wextra -Werror -O3 -D _USE_OMP_ -D NB_THREADS=16

# Builds everything.
all: 
	mkdir -p bin
	cd $(SRCDIR) && $(MAKE) all

# Cleans compilation files.
clean:
	cd $(SRCDIR) && $(MAKE) clean

run:
	/usr/local/k1tools/bin/k1-jtag-runner --progress --multibinary=$(CURDIR)/bin/dna.mpk --exec-multibin=IODDR0:$(CURDIR)/bin/master
