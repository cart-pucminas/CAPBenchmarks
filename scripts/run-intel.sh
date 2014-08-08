#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Directories.
export ROOTDIR=$PWD
export RESULTSDIR=$ROOTDIR/results
export BINDIR=$ROOTDIR/bin

# Create results directory.
mkdir -p $RESULTSDIR

for kernel in fast fn gf is km lu tsp; do
	echo "running $kernel"
	
	# Weak scaling
	echo "running weak scaling test: iteration $it"
	for nprocs in 1 2 4 8 16; do
		for class in tiny small large huge; do
			likwid-powermeter $BINDIR/$kernel.intel --verbose --class $class --nclusters $nprocs &>> $RESULTSDIR/$kernel-$class-$nprocs.mppa
		done
	done
	
	# Strong scaling.
	for it in {1..10}; do
		echo "running strong scaling test: iteration $it"
		for nprocs in 1 2 3 5 6 7 8 9 10 11 12 13 14 15 16; do
			likwid-powermeter $BINDIR/$kernel.intel --verbose --class $class --nclusters $nprocs &>> $RESULTSDIR/$kernel-$class-$nprocs.mppa
		done
	done
done


