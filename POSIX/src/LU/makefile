#
# Copyright(C) 2019 Guilherme Reis Barbosa de Oliveira <grboliveira@sga.pucminas.br>
#
# lu.c - Lower Upper Benchmark Kerkernl.
#

# Source files.
SRC = $(wildcard *.c)

# Executable file.
EXEC = lu

# Builds kernel.
all: $(SRC)
	$(CC) $(SRC) $(CFLAGS) -o $(BINDIR)/$(EXEC).intel $(LIBS)
	
# Cleans compilation files.
clean:
	rm -f $(BINDIR)/$(EXEC).intel
