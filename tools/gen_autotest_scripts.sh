#!/bin/bash

# usage: cd <stress_test-install> && gen_autotest_scripts.sh

SCRIPT_PATH=$(readlink -m "${BASH_SOURCE[0]}")
SCRIPT_NAME="${SCRIPT_PATH##*/}"
AUTOTEST_HOME="${SCRIPT_PATH%/*/*/*/*}"
scriptdir="$AUTOTEST_HOME/data/scripts"
rm -f $scriptdir/stress-*

while IFS=\| read script_bn commands; do
    script=$scriptdir/$script_bn

    cat >> $script <<'EOF'
#!/usr/bin/bash

SCRIPT_PATH=$(readlink -m "${BASH_SOURCE[0]}")
SCRIPT_NAME="${SCRIPT_PATH##*/}"
AUTOTEST_HOME="${SCRIPT_PATH%/*/*/*}"
. $AUTOTEST_HOME/bin/config.sh

recorddir=$WORKDIR/output/$SCRIPT_NAME
EOF

    cat >> $script <<EOF
commands="$commands"
EOF

    cat >> $script <<'EOF'
. $AUTOTEST_HOME/stress_test/install/bin/run.sh
EOF

chmod +x $script
done < ./testcases
