#
# Copyright(C) 2013-2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Builds everything.
all: release debug

# Builds release version.
release:
	cd array && $(MAKE) release
	cd globl && $(MAKE) release
	cd object && $(MAKE) release
	cd util && $(MAKE) release
	cd vector && $(MAKE) release

# Cleans compilation files.
clean:
	cd array && $(MAKE) clean
	cd globl && $(MAKE) clean
	cd object && $(MAKE) clean
	cd util && $(MAKE) clean
	cd vector && $(MAKE) clean
	rm -f $(LIBDIR)/$(LIB)
