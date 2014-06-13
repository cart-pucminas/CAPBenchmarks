#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Source files.
SRC_MASTER = $(wildcard master/*.c)
          
SRC_SLAVE =  $(wildcard slave/*.c)

# Executable.
EXEC = lu

.PHONY: master
.PHONY: slave

# Builds kernel.
all: master slave
	$(MPPADIR)/bin/createImage.rb --toolchain $(MPPADIR) --clusters=$(EXEC).slave --boot=$(EXEC).master -T $(BINDIR)/$(EXEC).mppa.mpk
	rm -f $(EXEC).master
	rm -f $(EXEC).slave

# Builds master.
master: $(SRC_MASTER)
	$(CC) -mos=rtems $(CFLAGS) $(SRC_MASTER) $(LIBSRCDIR)/*.c -o $(EXEC).master $(LIBS)

# Builds slave.
slave: $(SRC_SLAVE)
	$(CC) -mos=nodeos $(CFLAGS) $(SRC_SLAVE) $(LIBSRCDIR)/*.c -o $(EXEC).slave $(LIBS) -fopenmp
	
# Cleans compilation files.
clean:
	rm -f $(BINDIR)/$(EXEC).mppa.mpk
