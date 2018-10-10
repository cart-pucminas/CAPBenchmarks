#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Directories.
export BINDIR=bin
export K1DIR=/usr/local/k1tools/bin

# Default Parameters.
export CLASS=tiny
export NPROCS=2

echo "Problem size = $CLASS"

for kernel in lu; #fn
do
	echo "  ========== Running $kernel kernel"
	$K1DIR/k1-jtag-runner                               \
		--multibinary=$BINDIR/$kernel.img               \
		--exec-multibin=IODDR0:io_bin                   \
		-- --verbose --class $CLASS --nclusters $NPROCS
done
