#!/bin/sh

BINDIR=`dirname $0`
. $BINDIR/config.sh


LEFT=
RIGHT=

while [ $# -gt 0 ]; do
    if [ x$1 = "x-" ]; then
	shift
	break;
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


timeout $TIMEOUT $MCKDIR/sbin/ihkosctl 0 kmsg > /tmp/dtest-kmsg.log

if [ $? -eq 0 ]; then
    echo SUCCESS kmsg
else
    echo FAIL kmsg
    exit -1
fi

timeout $TIMEOUT sudo $MCKDIR/sbin/ihkosctl 0 clear_kmsg

if [ $? -eq 0 ]; then
    echo SUCCESS clear_kmsg
else
    echo FAIL clear_kmsg
    exit -1
fi

timeout $TIMEOUT $WRAPPER $MCKDIR/bin/mcexec $COMMAND 1> /tmp/dtest.log 2>&1

if [ $? -eq 0 ]; then
    echo SUCCESS mcexec
else
    echo FAIL mcexec
    exit -1
fi

fgrep FAIL /tmp/dtest.log

if [ $? -eq 1 ]; then
    echo SUCCESS $WRAPPER mcexec $COMMAND
else
    echo FAIL $WRAPPER mcexec $COMMAND
    exit -1
fi

timeout $TIMEOUT $MCKDIR/sbin/ihkosctl 0 kmsg > /tmp/dtest-kmsg.log

if [ $? -eq 0 ]; then
    echo SUCCESS kmsg
else
    echo FAIL kmsg
    exit -1
fi

NUMMSG=`echo /tmp/dtest-kmsg.log | wc -l`

if [ $NUMMSG -eq 1 ]; then
    echo SUCCESS kmsg $NUMMSG lines
else
    echo FAIL kmsg $NUMMSG lines
    exit -1
fi

timeout $TIMEOUT $MCKDIR/sbin/ihkosctl 0 ioctl 40000000 1

if [ $? -eq 0 ]; then
    echo SUCCESS ioctl 40000000 1
else
    echo FAIL ioctl 40000000 1
    exit -1
fi

timeout $TIMEOUT $MCKDIR/sbin/ihkosctl 0 kmsg > /tmp/dtest-process.log

if [ $? -eq 0 ]; then
    echo SUCCESS kmsg
else
    echo FAIL kmsg
    exit -1
fi

NUMPROCESSES=`awk '$4=="processes"{print $3}' /tmp/dtest-process.log`

if [ $NUMPROCESSES -eq 0 ]; then
    echo SUCCESS $NUMPROCESSES processes found
else
    echo FAIL $NUMPROCESSES processes found
fi

timeout $TIMEOUT $MCKDIR/sbin/ihkosctl 0 ioctl 40000000 2

if [ $? -eq 0 ]; then
    echo SUCCESS ioctl 40000000 2
else
    echo FAIL ioctl 40000000 2
    exit -1
fi

timeout $TIMEOUT $MCKDIR/sbin/ihkosctl 0 kmsg > /tmp/dtest-threads.log

if [ $? -eq 0 ]; then
    echo SUCCESS kmsg
else
    echo FAIL kmsg
    exit -1
fi

NUMTHREADS=`awk '$4=="processes"{print $3}' /tmp/dtest-threads.log`

if [ $NUMTHREADS -eq 0 ]; then
    echo SUCCESS $NUMTHREADS threads found
else
    echo FAIL $NUMTHREADS threads found
fi

if [ $NUMPROCESSES -ne 0 -a $NUMTHREADS -ne 0 ]; then
    exit -1
fi


