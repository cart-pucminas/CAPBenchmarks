#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# MPPA toolchain.
MPPADIR = /usr/local/k1tools
CC = $(MPPADIR)/bin/k1-gcc

# Toolchain configuration.
export CFLAGS = -Wall -Wextra -Werror

# Executable file.
EXEC = integer-sort

# Source files.
SRC_MASTER = master.c
SRC_SLAVE = slave.c

# Builds everything
all: master slave
	$(MPPADIR)/bin/createImage.rb --toolchain $(MPPADIR) --clusters=slave --boot=master -T $(EXEC).mpk

# Builds mppa master application
master: $(SRC_MASTER)
	$(CC) -mos=rtems $(CFLAGS) $(SRC_MASTER) -o master -lmppaipc

# Builds mppa master application
slave: $(SRC_SLAVE)
	$(CC) -mos=nodeos $(CFLAGS) $(SRC_SLAVE) -o slave -lmppaipc

# Cleans compilation files.
clean:
	rm -f master
	rm -f slave
	rm -f $(EXEC).mpk
