#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Directories.
export ROOTDIR=$PWD
export RESULTSDIR=$ROOTDIR/results
export BINDIR=$ROOTDIR/../bin

# Problem size.
export CLASS=tiny

# Compilation of all kernels
cd .. && make all-mppa256

# Create results directory.
mkdir -p $RESULTSDIR

echo "Problem size = $CLASS"

for kernel in fn; do
	for nprocs in {1..16}; do
	    echo "  >> running $kernel with $nprocs cluster(s)."
	    k1-jtag-runner --multibinary=$BINDIR/$kernel.img --exec-multibin=IODDR0:io_bin -- --verbose --class $CLASS --nclusters $nprocs &>> $RESULTSDIR/$kernel-$CLASS-$nprocs.mppa
	done
done
