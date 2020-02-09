#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Directories.
export BINDIR  = $(CURDIR)/bin

# Builds all kernels for Intel x86.
all-x86:
	mkdir -p bin
	cd x86 && $(MAKE) all BINDIR=$(BINDIR)

# Builds all kernels for POSIX
all-posix:
	mkdir -p bin
	cd posix && $(MAKE) all BINDIR=$(BINDIR)

# Builds all kernels (using async library) for MPPA-256.
all-mppa256-async:
	mkdir -p bin
	cd mppa256 && $(MAKE) all-async BINDIR=$(BINDIR)

# Builds all kernels (using ipc library) for MPPA-256.
all-mppa256-ipc:
	mkdir -p bin
	cd mppa256 && $(MAKE) all-ipc BINDIR=$(BINDIR)

# Builds all kernels for Gem5 Simulator
# IMPORTANT: Must use a compatible Kernel
all-gem5:
	mkdir -p bin
	cd gem5 && $(MAKE) all BINDIR=$(BINDIR)

# Cleans compilation files.
clean: clean-x86 clean-mppa256 clean-gem5 clean-posix

clean-x86:
	cd x86 && $(MAKE) clean BINDIR=$(BINDIR)
clean-mppa256:
	cd mppa256 && $(MAKE) clean BINDIR=$(BINDIR)
clean-gem5:
	cd gem5 && $(MAKE) clean BINDIR=$(BINDIR)
clean-posix:
	cd posix && $(MAKE) clean BINDIR=$(BINDIR)
