#!/bin/sh

cd ${AUTOTEST_HOME}/stress/bin
rm -f ${WORKDIR}/result.log
@TESTCASE@
echo $? > ${WORKDIR}/result.log
