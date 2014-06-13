#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#
# Builds CAP Benchmark suite for x86 architecture.
#

# Directories.
export BINDIR    = $(CURDIR)/bin
export INCDIR    = $(CURDIR)/include
export LIBDIR    = $(CURDIR)/lib
export LIBSRCDIR = $(CURDIR)/libsrc
export SRCDIR    = $(CURDIR)/src

# Tool chain.
export CC = gcc
export AR = ar

# Tool chain configuration.
export CFLAGS = -fno-builtin -Wall -Wextra -Werror -O3
export CFLAGS += -I $(INCDIR)
export ARFLAGS = -vq

# Libraries.
export LIB = libcapb.a
export LIBS = -lm -fopenmp $(LIBDIR)/$(LIB)

.PHONY: lib

# Builds all kernels.
all: lib
	cd $(SRCDIR) && $(MAKE) all
	
# Builds kernel library.
lib:
	cd $(LIBSRCDIR) && $(MAKE) all
	
# Cleans compilation files.
clean:
	cd $(LIBSRCDIR) && $(MAKE) clean
	cd $(SRCDIR) && $(MAKE) clean
