#!/bin/sh

BINDIR=`dirname $0`
. $BINDIR/config.sh

timeout $TIMEOUT sudo $MCKDIR/sbin/mcreboot.sh

if [ $? -eq 0 ]; then
    echo SUCCESS mcreboot
    exit 0
else
    echo FAIL mcreboot
    exit -1
fi
