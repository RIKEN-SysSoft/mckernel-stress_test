#!/bin/sh

BINDIR=`dirname $0`
. $BINDIR/config.sh

timeout -s 9 $TIMEOUT sudo $MCKDIR/sbin/mcreboot.sh

if [ $? -eq 0 ]; then
    echo SUCCESS mcreboot
    sudo chmod 777 /dev/mcos0
    exit 0
else
    echo FAIL mcreboot
    exit -1
fi
