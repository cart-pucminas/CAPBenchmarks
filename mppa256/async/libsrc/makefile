#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# OBS: MAKEFILE NAO INTERFERINDO NA EXECUÇÃO ??
	
# Source files.
SRC = $(wildcard *.c)

# Object files.
OBJ_MASTER = util.master.o problem.master.o async_util.master.o timer.master.o
OBJ_SLAVE = util.slave.o problem.slave.o async_util.slave.o timer.slave.o

# Builds all libraries.
all: master-lib slave-lib

# Builds master library.
master-lib: $(OBJ_MASTER)
	$(AR) $(ARFLAGS) $(LIBDIR)/$(LIBNAME).master.a $(OBJ_MASTER)

# Builds slave library.
slave-lib: $(OBJ_SLAVE)
	$(AR) $(ARFLAGS) $(LIBDIR)/$(LIBNAME).slave.a $(OBJ_SLAVE)

%.master.o: %.c
	$(CC) -mos=rtems $(CFLAGS) $< -c -o $@

%.slave.o: %.c
	$(CC) -mos=rtems $(CFLAGS) $< -c -o $@
	
# Cleans compilation files.
clean:
	rm -f $(OBJ_SLAVE)
	rm -f $(OBJ_MASTER)
	rm -f $(LIBDIR)/$(LIBNAME).slave.a
	rm -f $(LIBDIR)/$(LIBNAME).master.a
