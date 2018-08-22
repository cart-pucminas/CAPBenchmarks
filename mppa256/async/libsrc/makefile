#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# OBS: MAKEFILE NAO INTERFERINDO NA EXECUÇÃO ??
	
# Source files.
SRC = $(wildcard *.c)

# Object files.
OBJ_MASTER = problem.master.o async_util.master.o spawn_util.master.o infos.master.o timer.master.o
OBJ_SLAVE = problem.slave.o async_util.slave.o spawn_util.slave.o infos.slave.o timer.slave.o

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
