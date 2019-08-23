#!/bin/sh

LEFT=
RIGHT=
KMSGKW=

while [ $# -gt 0 ]; do
    if [ x$1 = "x-" ]; then
	shift
	break;
    fi
    if [ x$1 = "x-kmsgkw" ]; then
	shift
	KMSGKW=$1
	shift
	continue;
    fi
    LEFT="$LEFT $1"
    shift
done

while [ $# -gt 0 ]; do
    if [ x$1 = "x-" ]; then
	shift
	break;
    fi
    RIGHT="$RIGHT $1"
    shift
done

if [ "x$RIGHT" = "x" ]; then
    RIGHT=$LEFT
    LEFT=
fi

WRAPPER=$LEFT
COMMAND=$RIGHT

sleep 2
timeout -s 9 $STRESS_TEST_TIMEOUT $MCKINSTALL/sbin/ihkosctl 0 kmsg > $recorddir/dtest-kmsg.log

if [ $? -ne 0 ]; then
    echo "[ NG ] kmsg failed"
    exit 1
fi

timeout -s 9 $STRESS_TEST_TIMEOUT sudo $MCKINSTALL/sbin/ihkosctl 0 clear_kmsg

if [ $? -ne 0 ]; then
    echo "[ NG ] clear_kmsg failed"
    exit 1
fi

ulimit -S -c 0

echo "timeout -s 9 $STRESS_TEST_TIMEOUT $WRAPPER $MCKINSTALL/bin/mcexec $COMMAND 1> $recorddir/dtest.log 2>&1"
timeout -s 9 $STRESS_TEST_TIMEOUT $WRAPPER $MCKINSTALL/bin/mcexec $COMMAND 1> $recorddir/dtest.log 2>&1

ret=$?
if [ $ret -eq 0 ]; then
    echo "[ OK ] no error reported with the exit status"
else
    echo "[ NG ] error reported with the exit status ($ret), the log follows:"
    echo "=== log begins ==="
    cat $recorddir/dtest.log
    echo "=== log ends ==="
    exit 1
fi

if ! grep -E '(\[ NG \]|FAIL)' $recorddir/dtest.log > /dev/null; then
    echo "[ OK ] no error reported in the log"
else
    echo "[ NG ] error reported in the log:"
    echo "=== log begins ==="
    cat $recorddir/dtest.log
    echo "=== log ends ==="
    exit -1
fi

sleep 3
timeout -s 9 $STRESS_TEST_TIMEOUT $MCKINSTALL/sbin/ihkosctl 0 kmsg > $recorddir/dtest-kmsg.log

if [ $? -ne 0 ]; then
    echo "[ NG ] kmsg failed"
    exit -1
fi

if [ x"$KMSGKW" != "x" ]; then
    grep "$KMSGKW" $recorddir/dtest-kmsg.log > /dev/null 2>&1
    if [ $? -eq 0 ]; then
	echo [ OK ] keyword "$KMSGKW" found in kmsg
    else
	echo [ NG ] keyword "$KMSGKW" not found in kmsg
	cat $recorddir/dtest-kmsg.log
	exit 1
    fi
else
    if [ "$testname" != "signalonfork" ] &&  [ "$testname" != "signalonread" ]; then
	NUMMSG=`cat $recorddir/dtest-kmsg.log | wc -l`
	if [ "$NUMMSG" -ne 1 ]; then
	    echo "[INFO] warning: kmsg isn't empty"
	fi
    fi
fi

timeout -s 9 $STRESS_TEST_TIMEOUT $MCKINSTALL/sbin/ihkosctl 0 ioctl 40000000 1

if [ $? -ne 0 ]; then
    echo "[ NG ] showing number of remaining processes failed"
    exit 1
fi

sleep 3
timeout -s 9 $STRESS_TEST_TIMEOUT $MCKINSTALL/sbin/ihkosctl 0 kmsg > $recorddir/dtest-process.log

if [ $? -ne 0 ]; then
    echo "[ NG ] kmsg failed"
    exit 1
fi

NUMPROCESSES=`awk '$4=="processes"{print $3}' $recorddir/dtest-process.log`

if [ "$NUMPROCESSES" == "0" ]; then
    echo "[ OK ] no processes remaining"
else
    echo "[ NG ] $NUMPROCESSES processes remaining"
    exit 1
fi

timeout -s 9 $STRESS_TEST_TIMEOUT $MCKINSTALL/sbin/ihkosctl 0 ioctl 40000000 2

if [ $? -ne 0 ]; then
    echo "[ NG ] showing number of remaining threads failed"
    exit 1
fi

sleep 3
timeout -s 9 $STRESS_TEST_TIMEOUT $MCKINSTALL/sbin/ihkosctl 0 kmsg > $recorddir/dtest-threads.log

if [ $? -ne 0 ]; then
    echo [ NG ] kmsg failed
    exit 1
fi

NUMTHREADS=`awk '$4=="threads"{print $3}' $recorddir/dtest-threads.log`

if [ "$NUMTHREADS" == "0" ]; then
    echo "[ OK ] no threads remaining"
else
    echo "[ NG ] $NUMTHREADS threads remaining"
    exit 1
fi

pidof_mcexec=`pidof mcexec`
if [ "$pidof_mcexec" == "" ]; then
    echo "[ OK ] no mcexec running"
else
    echo "[ NG ] mcexec is remaining"
    exit 1
fi
