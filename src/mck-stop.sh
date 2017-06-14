#!/bin/sh

BINDIR=`dirname $0`
. $BINDIR/config.sh

timeout $TIMEOUT sudo $MCKDIR/sbin/mcstop+release.sh

if [ $? -eq 0 ]; then
    echo SUCCESS mcstop+release
    exit 0
else
    echo FAIL mcstop+release
    exit -1
fi
