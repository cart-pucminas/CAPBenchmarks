#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Builds all kernels.
all: gf #km lu fn

# Builds FN kernel.
fn:
	cd FN && $(MAKE) all
	cp FN/output/bin/*.img $(BINDIR)

# Builds LU kernel.
lu:
	cd LU && $(MAKE) all
	cp LU/output/bin/*.img $(BINDIR)

# Builds KM kernel.
km:
	cd KM && $(MAKE) all
	cp KM/output/bin/*.img $(BINDIR)

# Builds GF kernel.
gf:
	cd GF && $(MAKE) all
	cp GF/output/bin/*.img $(BINDIR)

# Cleans compilation files.
clean:
	cd FN && $(MAKE) clean
	cd KM && $(MAKE) clean
	cd LU && $(MAKE) clean
	cd GF && $(MAKE) clean
	rm -f $(BINDIR)/*.img