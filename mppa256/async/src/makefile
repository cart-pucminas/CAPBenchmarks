#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Builds all kernels.
all: is #fast gf km lu fn

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

# Builds FAST kernel
fast:
	cd FAST && $(MAKE) all
	cp FAST/output/bin/*.img $(BINDIR)

# Builds IS kernel.
is:
	cd IS && $(MAKE) all
	cp IS/output/bin/*.img $(BINDIR)

# Cleans compilation files.
clean:
	cd FN && $(MAKE) clean
	cd KM && $(MAKE) clean
	cd LU && $(MAKE) clean
	cd GF && $(MAKE) clean
	cd FAST && $(MAKE) clean
	cd IS && $(MAKE) clean
	rm -f $(BINDIR)/*.img