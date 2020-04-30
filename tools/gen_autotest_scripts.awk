#!/bin/awk -f

# usage: cd <stress_test-install> && awk -f gen_autotest_scripts.awk testcases

BEGIN { 
    FS="|"
    "dirname " ARGV[0] | getline dir;
    "cd " dir "/../../.. && pwd -P" | getline autotest_home;
    "cd " dir "/.. && pwd -P" | getline stress_test_install;

    scriptdir = sprintf("%s/data/scripts", autotest_home); 
    system("rm -f " scriptdir "/stress-*");
}

!/^#|^$/ {
    script_bn = $1;
    commands = $2;
    script = sprintf("%s/%s", scriptdir, script_bn);

    print "#!/bin/sh\n"  > script;

    print "# Define WORKDIR, DATADIR, MCKINSTALL etc." >> script;
    printf(". %s/bin/config.sh\n", autotest_home) >> script;

    printf("recorddir=$WORKDIR/output/%s\n", script_bn) >> script;

    printf("commands='%s'\n\n", commands) >> script;
    printf(". %s/bin/run.sh", stress_test_install) >> script;

    system("chmod +x " script);
}
