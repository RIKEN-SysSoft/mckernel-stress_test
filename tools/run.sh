#!/bin/sh

# It's sourced from /work/mcktest/data/scripts/stress-* scripts.

SCRIPT_PATH=$(readlink -m "${BASH_SOURCE[0]}")
AUTOTEST_HOME="${SCRIPT_PATH%/*/*/*/*}"
STRESS_TEST_DIR=$AUTOTEST_HOME/stress_test/install

error() {
    echo $1
    exit 1
}

ihkosctl=$MCKINSTALL/sbin/ihkosctl

# Prepare working directory
rm -rf $recorddir && mkdir -p $recorddir && pushd $recorddir > /dev/null || error "[ NG ] $recorddir not found"

# Prepare files etc.

$ihkosctl 0 clear_kmsg || exit $?

old_ulimit=$(ulimit -c)
ulimit -S -c 0

# e.g. extract "segviolation" from "stress-1-1-segviolation-001"
testname=$(basename $0)
testname=${testname%-*}
testname=${testname##*-}

if [ -e $STRESS_TEST_DIR/bin/init/${testname}.sh ]; then
    . $STRESS_TEST_DIR/bin/init/${testname}.sh
fi

# Parse
keyword=$(echo $commands | awk 'BEGIN {FS=","} {print $1}')
linuxprog=$(echo $commands | awk 'BEGIN {FS=","} {print $2}')
mckprog=$(echo $commands | awk 'BEGIN {FS=","} {print $3}')

# Run test
eval "timeout -s 9 $STRESS_TEST_TIMEOUT $STRESS_TEST_DIR/bin/$linuxprog $MCKINSTALL/bin/mcexec $STRESS_TEST_DIR/bin/$mckprog" > $recorddir/dtest.log 2>&1
ret=$?

# Clean up files etc.

ulimit -S -c $old_ulimit

if [ -e $STRESS_TEST_DIR/bin/fini/${testname}.sh ]; then
    . $STRESS_TEST_DIR/bin/fini/${testname}.sh
fi
popd > /dev/null

# OK/NG decision
[[ "$ret" != "0" ]] && error "[ NG ] exit status: $ret"

grep -E '(\[ NG \]|FAIL)' $recorddir/dtest.log > /dev/null && error "[ NG ] test program reported error"

timeout -s 9 $STRESS_TEST_TIMEOUT $MCKINSTALL/sbin/ihkosctl 0 kmsg > $recorddir/dtest-kmsg.log

if [[ "$keyword" != "" ]]; then
    ! grep -q "$keyword" $recorddir/dtest-kmsg.log && error "[ NG ] keyword $keyword not found in kmsg"
else
    if [[ "$testname" != "signalonfork" ]] && [[ "$testname" != "signalonread" ]]; then
	[[ "$(cat $recorddir/dtest-kmsg.log | wc -l)" != 1 ]] && echo "[WARN] kmsg isn't empty"
    fi
fi

# Check if process/thread structs remain
show_struct_process_or_thread="$recorddir/show_struct_process_or_thread.log"
$ihkosctl 0 clear_kmsg
$ihkosctl 0 ioctl 40000000 1
$ihkosctl 0 ioctl 40000000 2
$ihkosctl 0 kmsg > $show_struct_process_or_thread

nprocs=`awk '$4=="processes"{print $3}' $show_struct_process_or_thread`
[[ "$nprocs" != "0" ]] && error "[ NG ] $nprocs process(es) remaining"

nthreads=`awk '$4=="threads"{print $3}' $show_struct_process_or_thread`
[[ "$nthreads" != "0" ]] && error "[ NG ] $nprocs thread(s) remaining"

[[ "$(pidof mcexec)" != "" ]] && error "[ NG ] mcexec is remaining"

exit 0
