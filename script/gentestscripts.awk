#!/bin/awk -f

# usage: awk -f gentestscripts.awk testcases

BEGIN {
    autotest_home = "/work/mcktest";
}

{
    script = autotest_home "/data/script/" $1;
    command = $2;
    for (i = 3; i <= NF; i++) {
	command = command " " $i;
    }
    system("sed -e 's#@TESTCASE@#" command "#' run-template.sh > " script);
    system("chmod +x " script);
}
