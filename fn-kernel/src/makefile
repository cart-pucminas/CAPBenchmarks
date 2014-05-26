#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Source files.
X86_SRC = $(wildcard *.c)     \
          $(wildcard x86/*.c) \

# Source files.
MPPA_SRC_MASTER = $(wildcard *.c)             \
                  $(wildcard mppa/*.c)        \
                  $(wildcard mppa/master/*.c) \
          
MPPA_SRC_SLAVE =  $(wildcard mppa/*.c)       \
                  $(wildcard mppa/slave/*.c) \

.PHONY: mppa
.PHONY: x86

# Builds Intel x86.
x86: $(X86_SRC)
	$(CC) $(X86_SRC) $(CFLAGS) -I $(INCDIR) -D_INTEL_I7_ -o $(BINDIR)/$(EXEC).intel $(LIBS)

# Builds for MPPA-256.
mppa256: master slave
	$(MPPADIR)/bin/createImage.rb --toolchain $(MPPADIR) --clusters=$(EXEC).slave --boot=$(EXEC).master -T $(BINDIR)/$(EXEC).mppa.mpk
	rm -f $(EXEC).master
	rm -f $(EXEC).slave

# Builds mppa master application
master: $(MPPA_SRC_MASTER)
	$(K1CC) -mos=rtems $(CFLAGS) -I $(INCDIR) -D_MPPA_256_ $(MPPA_SRC_MASTER) -o $(EXEC).master $(LIBS)

# Builds mppa master application
slave: $(MPPA_SRC_SLAVE)
	$(K1CC) -mos=nodeos $(CFLAGS) -I $(INCDIR) -D_MPPA_256_ $(MPPA_SRC_SLAVE) -o $(EXEC).slave $(LIBS) -fopenmp
	
# Cleans compilation files.
clean:
	rm -f $(BINDIR)/$(EXEC).intel
	rm -f $(BINDIR)/$(EXEC).mppa
