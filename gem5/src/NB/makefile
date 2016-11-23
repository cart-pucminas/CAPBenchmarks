#
# Copyright(C) 2016 Matheus Alcântara Souza <matheusalcantarasouza@gmail.com>
#

# Source files.
SRC = $(wildcard *.c)

# Executable file.
EXEC = nb

# Builds kernel.
all: $(SRC)
	$(CC) $(SRC) $(CFLAGS) -o $(BINDIR)/$(EXEC) $(LIBS)
	
# Cleans compilation files.
clean:
	rm -f $(BINDIR)/$(EXEC)