#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#
# Builds CAP Benchmark suite for MPPA-256 architecture.
#

# Problem size.

# Builds kernel usync async.
all-async:
	cd async && $(MAKE) all

# Builds kernel usync ipc.
all-ipc:
	cd ipc && $(MAKE) all
	
# Cleans compilation files.
clean:
	cd async && $(MAKE) clean
