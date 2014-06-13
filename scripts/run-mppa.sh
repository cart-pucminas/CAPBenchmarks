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

for kernel in fn gf is km lu tsp; do
	for it in {1..10}; do
		echo "running $kernel iteration $it"
		# Weak scaling.
		$MPPADIR/bin/k1-power -- $MPPADIR/bin/k1-jtag-runner --multibinary=$BINDIR/$kernel.mppa.mpk --exec-multibin=IODDR0:$kernel.master -- --verbose --class tiny --nthreads 1 &>> $RESULTSDIR/$kernel-tiny-1.mppa
		$MPPADIR/bin/k1-power -- $MPPADIR/bin/k1-jtag-runner --multibinary=$BINDIR/$kernel.mppa.mpk --exec-multibin=IODDR0:$kernel.master -- --verbose --class small --nthreads 2 &>> $RESULTSDIR/$kernel-small-2.mppa
		$MPPADIR/bin/k1-power -- $MPPADIR/bin/k1-jtag-runner --multibinary=$BINDIR/$kernel.mppa.mpk --exec-multibin=IODDR0:$kernel.master -- --verbose --class standard --nthreads 4 &>> $RESULTSDIR/$kernel-standard-4.mppa
		$MPPADIR/bin/k1-power -- $MPPADIR/bin/k1-jtag-runner --multibinary=$BINDIR/$kernel.mppa.mpk --exec-multibin=IODDR0:$kernel.master -- --verbose --class large --nthreads 8 &>> $RESULTSDIR/$kernel-large-8.mppa
		$MPPADIR/bin/k1-power -- $MPPADIR/bin/k1-jtag-runner --multibinary=$BINDIR/$kernel.mppa.mpk --exec-multibin=IODDR0:$kernel.master -- --verbose --class huge --nthreads 16 &>> $RESULTSDIR/$kernel-huge-16.mppa
	
		# Strong scaling.
		for nprocs in 1 2 3 5 6 7 8 9 10 11 12 13 14 15 16; do
			$MPPADIR/bin/k1-power -- $MPPADIR/bin/k1-jtag-runner --multibinary=$BINDIR/$kernel.mppa.mpk --exec-multibin=IODDR0:$kernel.master -- --verbose --class standard --nthreads $nprocs &>> $RESULTSDIR/$kernel-standard-$nprocs.mppa
		done
	done
done


