#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Executable.
EXEC = gf

# Sets the target platform.
platforms := hardware

# Compilation flags flags
cflags := -O3 -I $(INCDIR)
lflags := -lm
k1-lflags := -lmppaipc

# Builds master binary.
io-bin := io_bin
io_bin-cflags := -D_MASTER_
io_bin-srcs := $(wildcard master/*.c) $(wildcard $(LIBSRCDIR)/*.c)

# Builds slave binary
cluster-bin := cluster_bin
cluster_bin-lflags := -fopenmp
cluster_bin-srcs := $(wildcard slave/*.c) $(wildcard $(LIBSRCDIR)/*.c)

# build image (master + slave)
mppa-bin := multibin_bin
multibin_bin-objs := io_bin cluster_bin
multibin_bin-name := $(EXEC).img

# must be at the end of the makefile!
include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.mppaipc
