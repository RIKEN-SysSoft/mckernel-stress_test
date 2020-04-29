#!/bin/awk -f

# usage: awk -f gen_autotest_scripts.awk testcases

BEGIN { 
    FS="|"
    "pwd -P" | getline cwd;
    "dirname " ARGV[0] | getline dir;
    "cd " dir "/../.. && pwd -P" | getline autotest_home;

    scriptdir = sprintf("%s/data/scripts", autotest_home); 
    system("rm -f " scriptdir "/stress-*");
}

!/^#|^$/ {
    script_bn = $1;
    commands = $2;
    script = sprintf("%s/%s", scriptdir, script_bn);

    print "#!/bin/sh\n"  > script;

    print "# Define WORKDIR, DATADIR, MCKINSTALL etc." >> script;
    print ". ${AUTOTEST_HOME}/bin/config.sh\n" >> script;

    # Switch recorddir for McKernel run and Linux run

    recorddir_base = "$WORKDIR/output";
    printf("recorddir=%s/%s\n", recorddir_base, script_bn) >> script;

    printf("commands='%s'\n\n", commands) >> script;
    print(". ${AUTOTEST_HOME}/stress_test/util/run.sh") >> script;

    system("chmod +x " script);
}
