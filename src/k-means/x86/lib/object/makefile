#
# Copyright(C) 2013-2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Source files.
SRC = $(wildcard *.c)

# Object files.
OBJ = $(SRC:.c=.release.o)
OBJDBG = $(SRC:.c=.debug.o)

# Builds everything.
all: release debug

# Builds release version.
release: $(OBJ)
	$(AR) $(ARFLAGS) $(LIBDIR)/$(LIB) $(OBJ)

# Builds debug version.
debug: $(OBJDBG)
	$(AR) $(ARFLAGS) $(LIBDIR)/$(LIBDBG) $(OBJDBG)

# Builds release object files.
%.release.o: %.c
	$(CC) $< $(CFLAGS) $(OPTIMIZATION) -c -o $@

# Builds debug object files.
%.debug.o: %.c
	$(CC) $< $(CFLAGS) $(DEBUG) -c -o $@
	
# Cleans compilation files.
clean:
	rm -f $(OBJ)
	rm -f $(OBJDBG)
