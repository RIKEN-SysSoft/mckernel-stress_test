#!/bin/sh

# It's sourced from /work/mcktest/data/scripts/stress-* scripts.

ihkosctl=$MCKINSTALL/sbin/ihkosctl

# Prepare working directory
rm -rf $recorddir && mkdir -p $recorddir

# To check if kmsg will be empty
if [ "${linux_run}" != "yes" ]; then
	$ihkosctl 0 clear_kmsg
fi

# Run test
pushd STRESS_TEST_DIR/bin > /dev/null
eval $command_line
exit_code=$?
popd > /dev/null

# OK/NG decision
rc=0
if [ "${linux_run}" != "yes" ]; then

    if [ $exit_code != 0 ]; then
	rc=1
    fi

    # Check if kmsg is empty
    $ihkosctl 0 kmsg > $recorddir/kmsg.log
    if [ "`cat $recorddir/kmsg.log | wc -l`" -ne 1 ]; then
	echo "$(basename $0): WARNING: kmsg isn't empty."
    fi

    # Check if process/thread structs remain
    show_struct_process_or_thread="$recorddir/show_struct_process_or_thread.log"
    $ihkosctl 0 clear_kmsg
    $ihkosctl 0 ioctl 40000000 1
    $ihkosctl 0 ioctl 40000000 2
    $ihkosctl 0 kmsg > $show_struct_process_or_thread

    nprocs=`awk '$4=="processes"{print $3}' $show_struct_process_or_thread`
    if [ -n $nprocs ] && [ "$nprocs" != "0" ]; then
	echo "$(basename $0): INFO: $nprocs process(es) remaining"
	rc=1
    fi

    nthreads=`awk '$4=="threads"{print $3}' $show_struct_process_or_thread`
    if [ -n $nthreads ] && [ "$nthreads" != "0" ]; then
	echo "$(basename $0): INFO: $nprocs thread(s) remaining"
	rc=1
    fi
fi

exit $rc
