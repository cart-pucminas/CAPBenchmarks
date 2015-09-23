#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Directories.
export ROOTDIR=$PWD
export RESULTSDIR=$ROOTDIR/results
export BINDIR=$ROOTDIR/bin
export MPPADIR=/usr/local/k1tools

# Create results directory.
mkdir -p $RESULTSDIR

for kernel in tsp; do
	echo "running $kernel"
	for class in tiny small standard large huge; do
	# Strong scaling.
	echo "running strong scaling test: iteration $it"
		$MPPADIR/bin/k1-jtag-runner \
		--multibinary=$BINDIR/$kernel.img --exec-multibin=IODDR0:master -- \
		--verbose --class $class --nclusters 16 # &>> $RESULTSDIR/$kernel-$class-16.mppa
	done
done


