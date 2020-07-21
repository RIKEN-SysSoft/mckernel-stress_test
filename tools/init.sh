#!/usr/bin/bash

SCRIPT_PATH=$(readlink -m "${BASH_SOURCE[0]}")
AUTOTEST_HOME="${SCRIPT_PATH%/*/*/*/*}"

. $AUTOTEST_HOME/bin/config.sh
. $AUTOTEST_HOME/stress_test/install/bin/util.sh

backup_mck
patch_and_build "" ihk_kmsg_size
