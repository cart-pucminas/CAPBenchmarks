#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Builds all kernels.
all: lu fn

# Builds FN kernel.
fn:
	cd FN && $(MAKE) all
	cp FN/output/bin/*.img $(BINDIR)

# Builds LU kernel.
lu:
	cd LU && $(MAKE) all
	cp LU/output/bin/*.img $(BINDIR)

# Cleans compilation files.
clean:
	cd FN && $(MAKE) clean
	cd LU && $(MAKE) clean
	rm -f $(BINDIR)/*.img
