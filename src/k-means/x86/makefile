#
# Copyright(C) 2013-2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Directories.
export BINDIR  = $(CURDIR)/bin
export INCDIR  = $(CURDIR)/include
export LIBDIR  = $(CURDIR)/lib
export SRCDIR  = $(CURDIR)/src

# Executable files.
export EXEC = dna
export EXECDBG = dna.debug

# Library files.
export LIB = libc.a
export LIBDBG = libc.debug.a

# Toolchain.
export CC = gcc
export AR = ar
# Toolchain configuration.
export OPTMIZATION = -D NDEBUG - O3
export DEBUG = -Werror -g
export CFLAGS = -I $(INCDIR) -pedantic -Wall -Wextra -fopenmp
export ARFLAGS = -vq

# Builds everything for intel architecture.
all: intel.dna.release

# Builds intel release version.
intel.dna.release: library.release
	mkdir -p bin/
	cd $(SRCDIR) && $(MAKE) intel.release

# Builds library release version.
library.release:
	mkdir -p bin/
	cd $(LIBDIR) && $(MAKE) release

# Cleans compilation files.
clean:
	cd $(LIBDIR) && $(MAKE) clean
	cd $(SRCDIR) && $(MAKE) clean
