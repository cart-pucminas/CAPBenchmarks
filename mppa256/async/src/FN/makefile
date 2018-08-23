#
# Copyright (C) 2015-2017 Kalray SA. All rights reserved.
#
system-name := bare

cflags := -O3 -std=gnu99 -g -mhypervisor -I $(INCDIR) -Wall -Wextra -Winit-self -Wswitch-default -Wfloat-equal -Wundef -Wshadow -Wuninitialized #Werror

generalMPPAFlags := -g -lvbsp -lutask -lmppa_remote -lmppa_async -lmppa_request_engine -lmppapower -lmppanoc -lmpparouting -mhypervisor -Wl,--defsym=_LIBNOC_DISABLE_FIFO_FULL_CHECK=0

CLASS := tiny
nprocs := 1
nPES := 16
ARGS := --verbose --class ${CLASS} --nclusters ${nprocs} --nthreads ${nPES}

io-bin := io_bin
io_bin-cflags := -D_MASTER_ -D_FN_
io_bin-lflags := $(generalMPPAFlags) -lpcie_queue 
io_bin-srcs := $(wildcard master/*.c) $(wildcard $(LIBSRCDIR)/*.c)

cluster-bin := cluster_bin
cluster-cflags := -I .
cluster-system := $(cluster_system)
cluster-lflags := $(generalMPPAFlags)
cluster_bin-srcs := $(wildcard slave/*.c) $(wildcard $(LIBSRCDIR)/*.c)

mppa-bin := multibin_bin
multibin_bin-objs = io_bin cluster_bin

# clean hooks will be called with clean
clean-hooks := clean1

include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.kalray

all:
	${TRACE_CMD_PREFIX} $(K1_TOOLCHAIN_DIR)/bin/k1-jtag-runner ${JTAG_ARGS} --multibinary=./${O}/bin/${mppa-bin}.mpk --exec-multibin=IODDR0:${io-bin} -- ${ARGS}

clean1:
	rm -fr ${O} *~ MPPA.* bostan.*.trace* tracefile.* traces/ profile/ tracedtv/ callgrind/
