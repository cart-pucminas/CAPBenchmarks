# Executable.
EXEC = fast

# Use bare libraries
system-name := bare

# Compilation flags.
cflags := -fopenmp -O3 -std=gnu99 -g -mhypervisor -I $(INCDIR) -Wall -Wextra -Winit-self -Wswitch-default -Wshadow -Wuninitialized
MPPAFlags := -g -lvbsp -lutask -lgomp -lmppa_remote -lmppa_async -lmppa_request_engine -lmppapower -lmppanoc -lmpparouting -mhypervisor

# Builds master binary.
io-bin := io_bin
io_bin-cflags := -D_MASTER_ -D_FAST_
io_bin-lflags := $(MPPAFlags) -lpcie_queue -lm
io_bin-srcs := $(wildcard master/*.c) $(wildcard $(LIBSRCDIR)/*.c) 

# Builds slave binary.
cluster-bin := cluster_bin
cluster_bin-lflags := $(MPPAFlags) -lm
cluster_bin-srcs := $(wildcard slave/*.c) $(wildcard $(LIBSRCDIR)/*.c)

# Builds image (master + slave)
mppa-bin := multibin_bin
multibin_bin-objs := io_bin cluster_bin
multibin_bin-name := $(EXEC).img

# must be at the end of the makefile!
include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.kalray
