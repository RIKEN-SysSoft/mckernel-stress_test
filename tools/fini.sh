#!/usr/bin/bash

SCRIPT_PATH=$(readlink -m "${BASH_SOURCE[0]}")
AUTOTEST_HOME="${SCRIPT_PATH%/*/*/*/*}"

. $AUTOTEST_HOME/stress_test/install/bin/util.sh

restore_mck
