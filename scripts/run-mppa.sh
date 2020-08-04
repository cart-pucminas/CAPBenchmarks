#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Directories.
export BINDIR=bin
export K1DIR=/usr/local/k1tools/bin

# Default Parameters.
export KERNEL=$1
export CLASS=$2
export NPROCS=$3

echo "Problem size = $CLASS"

echo "  ========== Running $KERNEL kernel"
$K1DIR/k1-jtag-runner                        		\
	--multibinary=$BINDIR/$KERNEL.img               \
	--exec-multibin=IODDR0:io_bin                   \
	-- --verbose --class $CLASS --nclusters $NPROCS
