#
# Copyright(C) 2013-2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Source files.
SRC = $(wildcard *.c)

# Builds intel release version.
intel.release: $(SRC)
	$(CC) $(CFLAGS) $(OPTIMIZATION) $(SRC) -o $(BINDIR)/$(EXEC) $(LIBDIR)/$(LIB) -lm

# Builds intel debug version.
intel.debug: $(SRC)
	$(CC) $(CFLAGS) $(DEBUG) $(SRC) -o $(BINDIR)/$(EXECDBG) $(LIBDIR)/$(LIBDBG) -lm
	
	
# Cleans compilation files.
clean:
	rm -f $(BINDIR)/$(EXEC)
	rm -f $(BINDIR)/$(EXECDBG)
