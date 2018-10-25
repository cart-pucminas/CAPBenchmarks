#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Directories.
export BINDIR=bin
export ROOTDIR=$PWD
export RESULTSDIR=$ROOTDIR/results
export K1DIR=/usr/local/k1tools/bin

# Default Parameters.
export CLASS=standard

# Create results directory.
mkdir -p $RESULTSDIR

echo "Problem size = $CLASS"
for kernel in km lu fn;  do
	mkdir -p $RESULTSDIR/$kernel
	for nprocs in {1..16}; do
		if [ "$kernel" != "km" ] || [ "$nprocs" -ge 4 ]; then
			for repeat in {1..5}; do
				echo "  ========== Running $kernel with $nprocs cluster(s)."	
				$K1DIR/k1-jtag-runner                               \
					--multibinary=$BINDIR/$kernel.img               \
					--exec-multibin=IODDR0:io_bin                   \
					-- --verbose --class $CLASS --nclusters $nprocs \
					&>> $RESULTSDIR/$kernel/$kernel-$CLASS-$nprocs.mppa
			done
		fi
	done
done
