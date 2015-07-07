#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Source files.
SRC_COMMON = $(wildcard *.c)
SRC_MASTER = $(wildcard master/*.c)
SRC_SLAVE =  $(wildcard slave/*.c)

# Executable.
EXEC = tsp

# Debug mode? (use -DDEBUG to turn on debug mode or -UDEBUG to turn off debug mode)
DEBUG = -UDEBUG

.PHONY: master
.PHONY: slave

# Builds kernel.
all: master slave
	$(MPPADIR)/bin/k1-create-multibinary --toolchain $(MPPADIR) --clusters=$(EXEC).slave --boot=$(EXEC).master -T $(BINDIR)/$(EXEC).mppa.mpk
	rm -f $(EXEC).master
	rm -f $(EXEC).slave

# Builds common objects.
common: $(SRC_COMMON)
	$(CC) -Wno-unused-parameter -DMT -DNO_CACHE_COHERENCE $(DEBUG) -mos=rtems $(CFLAGS) $(SRC_COMMON) -c $(EXEC) $(LIBS)

# Builds master.
master: $(SRC_COMMON) $(SRC_MASTER)
	$(CC) -Wno-unused-parameter -DMT -DNO_CACHE_COHERENCE $(DEBUG) -mos=rtems $(CFLAGS) $(SRC_COMMON) $(SRC_MASTER) -o $(EXEC).master $(LIBS)

# Builds slave.
slave: $(SRC_COMMON) $(SRC_SLAVE) 
	$(CC) -Wno-unused-parameter -DMT -DNO_CACHE_COHERENCE $(DEBUG) -mos=nodeos $(CFLAGS) $(SRC_COMMON) $(SRC_SLAVE) -o $(EXEC).slave $(LIBS)

# Cleans compilation files.
clean:
	rm -f $(BINDIR)/$(EXEC).mppa.mpk
