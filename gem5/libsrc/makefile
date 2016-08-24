#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Object files.
OBJ = power.o timer.o util.o
GEM5OBJ = m5.o m5op_x86.o
GEM5FLAGS = -O2 #-DM5OP_ADDR=0xFFFF0000

# Builds library.
all: $(OBJ) $(GEM5OBJ)
	$(AR) $(ARFLAGS) $(LIBDIR)/$(LIB).a $(OBJ) $(GEM5OBJ)

# Builds object files.
%.o: %.c
	$(CC) $< $(CFLAGS) $(OPTIMIZATION) -c -o $@

m5op_x86.o:
	$(CC) m5op_x86.S $(GEM5FLAGS) -I$(INCDIR) -c -o m5op_x86.o

m5.o:
	$(CC) m5.c $(GEM5FLAGS) -I$(INCDIR) -c -o m5.o 

# Cleans compilation files.
clean:
	rm -f $(OBJ) $(GEM5OBJ)
	rm -f $(LIBDIR)/$(LIB).a
