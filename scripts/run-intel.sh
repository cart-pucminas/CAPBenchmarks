#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Parameters.
export ITERATIONS=5
export ROOTDIR=$PWD
export RESULTSDIR=$ROOTDIR/results
export BINDIR=$ROOTDIR/bin

# Create results directory.
rm -f $RESULTSDIR/*
mkdir -p $RESULTSDIR

# Iterations.
for it in {1..$ITERATIONS}; do
	# Kernels.
	for class in tiny small workstation standard large; do
		# Number of clusters.
		for kernel in fn gf is km lu; do
			# Classes.
			for nprocs in 1 2 4 8 16; do
				likwid-powermeter $BINDIR/$kernel.intel --verbose --class $class --nthreads $nprocs &>> $RESULTSDIR/$kernel-$class-$nprocs.intel
			done 
		done
	done
done
