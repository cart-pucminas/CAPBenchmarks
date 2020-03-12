#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Directories.
export BINDIR=bin
export K1DIR=/usr/local/k1tools/bin

echo Inform class size and number of clusters:
read classize nclusters

# Default Parameters.
export CLASS=$classize
export NPROCS=$nclusters

echo "Problem size = $CLASS"

for kernel in gf; #fn lu km gf fast is tsp;
do
	echo "  ========== Running $kernel kernel"
	$K1DIR/k1-jtag-runner                        		\
		--multibinary=$BINDIR/$kernel.img               \
		--exec-multibin=IODDR0:io_bin                   \
		-- --verbose --class $CLASS --nclusters $NPROCS
done