#!/bin/sh

# It's sourced from /work/mcktest/data/script/ostest-* scripts.

recorddir=$WORKDIR/output/$(basename $0)
rm -rf $recorddir && mkdir -p $recorddir && pushd $recorddir

testname=$(basename $0)
testname=${testname%-*}
testname=${testname##*-}

if [ -e $STRESS_TEST_DIR/bin/init/${testname}.sh ]; then
    . $STRESS_TEST_DIR/bin/init/${testname}.sh
fi

eval $command_line
exit_status=$?

if [ -e $STRESS_TEST_DIR/bin/fini/${testname}.sh ]; then
    . $STRESS_TEST_DIR/bin/fini/${testname}.sh
fi

popd

# OK/NG decision
rc=$exit_status
echo $rc > ${WORKDIR}/result.log

exit $rc
