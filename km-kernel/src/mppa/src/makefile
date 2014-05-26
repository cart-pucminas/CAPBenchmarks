#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Source files.
SRC_MASTER = master.c vector.c timer.c random.c
SRC_SLAVE = slave.c vector.c

# Builds everything
all: master slave
	rm -rf $(BINDIR)/$(EXEC).mpk
	cd $(BINDIR) &&	$(MPPADIR)/bin/createImage.rb --toolchain $(MPPADIR) --clusters=slave --boot=master -T $(EXEC).mpk

# Builds mppa master application
master: $(SRC_MASTER)
	$(CC) -mos=rtems $(CFLAGS) $(SRC_MASTER) -o $(BINDIR)/master -lmppaipc -lm

# Builds mppa master application
slave: $(SRC_SLAVE)
	$(CC) -mos=nodeos $(CFLAGS) $(SRC_SLAVE) -o $(BINDIR)/slave -lmppaipc -lm -fopenmp

# Cleans compilation files.
clean:
	rm -f $(BINDIR)/master
	rm -f $(BINDIR)/slave
	rm -f $(BINDIR)/$(EXEC).mpk
