# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Builds all kernels.
all: fn lu

# Builds FN kernel.
fn:
	cd FN && $(MAKE) all

# Builds LU kernel
lu:
	cd LU && $(MAKE) all


# Cleans compilation files.
clean:
	cd FN && $(MAKE) clean
	cd LU && $(MAKE) clean
