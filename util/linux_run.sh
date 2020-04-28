#!/bin/sh

linux_run="no"

while getopts H OPT
do
    case $OPT in
	H)
	    linux_run="yes"
	    ;;
    esac
done

shift `expr $OPTIND - 1`
