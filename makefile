#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Directories.
export BINDIR  = $(CURDIR)/bin
export MPPADIR = /usr/local/k1tools

# Tool chain.
export CC = gcc
export K1CC = $(MPPADIR)/bin/k1-gcc

# Tool chain configuration.
export CFLAGS = -fno-builtin -Wall -Wextra -Werror -O3

# Libraries.
export X86_LIBS = "-lm -fopenmp"
export MPPA_LIBS = "-lm -lmppaipc"

# Builds all kernels for Intel x86.
all-x86: fn-x86 is-x86 km-x86 lu-x86

# Builds FN kernel for Intel x86.
fn-x86:
	cd fn-kernel && $(MAKE) x86 LIBS=$(X86_LIBS)

# Builds IS kernel for x86.
is-x86:
	cd is-kernel && $(MAKE) x86 LIBS=$(X86_LIBS)

# Builds KM kernel for x86.
km-x86:
	cd km-kernel && $(MAKE) x86 LIBS=$(X86_LIBS)

# Builds LU kernel for x86.
lu-x86:
	cd lu-kernel && $(MAKE) x86 LIBS=$(X86_LIBS)

# Builds all kernels for MPPA-256.
all-mppa256: fn-mppa256 lu-mppa256

# Builds FN kernel for MPPA-256.
fn-mppa256:
	cd fn-kernel/ && $(MAKE) mppa256 LIBS=$(MPPA_LIBS)

# Builds KM kernel for MPPA-256.
km-mppa256:
	cd km-kernel/ && $(MAKE) mppa256 LIBS=$(MPPA_LIBS)

# Builds LU kernel for MPPA-256.
lu-mppa256:
	cd lu-kernel && $(MAKE) mppa256 LIBS=$(MPPA_LIBS)

# Cleans compilation files.
clean:
	cd fn-kernel && $(MAKE) clean
	cd is-kernel && $(MAKE) clean
	cd km-kernel && $(MAKE) clean
	cd lu-kernel && $(MAKE) clean