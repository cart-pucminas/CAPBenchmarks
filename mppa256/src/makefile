#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Builds all kernels.
all: fn gf is km lu

# Builds FN kernel.
fn:
	cd FN && $(MAKE) all

# Builds GF kernel.
gf:
	cd GF && $(MAKE) all

# Builds IS kernel.
is:
	cd IS && $(MAKE) all

# Builds KM kernel.
km:
	cd KM && $(MAKE) all

# Builds LU kernel.
lu:
	cd LU && $(MAKE) all

# Cleans compilation files.
clean:
	cd FN && $(MAKE) clean
	cd GF && $(MAKE) clean
	cd IS && $(MAKE) clean
	cd KM && $(MAKE) clean
	cd LU && $(MAKE) clean
