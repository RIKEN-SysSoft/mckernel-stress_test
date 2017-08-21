#!/bin/awk -f

BEGIN {
    autotest_home = "/work/mcktest";
    testlist = autotest_home "/data/stress-testlist";
    system("rm -f " testlist);
}

{
    script = autotest_home "/data/script/" $1;
    command = $2;
    for (i = 3; i <= NF; i++) {
	command = command " " $i;
    }
    system("sed -e 's#@TESTCASE@#" command "#' run-template.sh > " script);
    system("chmod +x " script);
    print $1 >> testlist
}
