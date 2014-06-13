#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#
# Builds CAP Benchmark suite for MPPA-256 architecture.
#

# Directories.
export BINDIR    = $(CURDIR)/bin
export INCDIR    = $(CURDIR)/include
export LIBDIR    = $(CURDIR)/lib
export LIBSRCDIR = $(CURDIR)/libsrc
export MPPADIR   = /usr/local/k1tools
export SRCDIR    = $(CURDIR)/src

# Tool chain.
export AR = $(MPPADIR)/bin/k1-ar
export CC = $(MPPADIR)/bin/k1-gcc

# Tool chain configuration.
export CFLAGS = -Wall -Wextra -O3
export CFLAGS += -I $(INCDIR)
export ARFLAGS = -vq

# Libraries.
export LIBNAME = libcapb
export LIBS = -lm -lmppaipc

.PHONY: lib

# Builds all kernels.
all:
	cd $(SRCDIR) && $(MAKE) all

# Builds libraries.
lib:
	cd $(LIBSRCDIR) && $(MAKE) all
	
# Cleans compilation files.
clean:
	cd $(SRCDIR) && $(MAKE) clean
	cd $(LIBSRCDIR) && $(MAKE) clean
