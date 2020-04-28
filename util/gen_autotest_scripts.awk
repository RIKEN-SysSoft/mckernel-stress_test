#!/bin/awk -f

# usage: awk -f gen_autotest_scripts.awk testcases

BEGIN { 
    "pwd -P" | getline cwd;
    "dirname " ARGV[0] | getline dir;
    "cd " dir "/../.. && pwd -P" | getline autotest_home;

    scriptdir = sprintf("%s/data/scripts", autotest_home); 
    system("rm " scriptdir "/stress-*");
}

!/^#|^$/ {
    script_bn = $1;
    command_line = $2;
    for (i = 3; i <= NF; i++) {
	command_line = command_line " " $i;
    }
    script = sprintf("%s/%s", scriptdir, script_bn);

    print "#!/bin/sh\n"  > script;

    print "# Define linux_run" >> script;
    print ". ${AUTOTEST_HOME}/stress_test/util/linux_run.sh\n" >> script;

    print "# Define WORKDIR, DATADIR, MCKINSTALL etc." >> script;
    print ". ${AUTOTEST_HOME}/bin/config.sh\n" >> script;

    # Switch recorddir for McKernel run and Linux run

    printf("if [ \"${linux_run}\" != \"yes\" ]; then\n") >> script;

    recorddir_base = "$WORKDIR/output";
    printf("\trecordfile=%s/%s.output\n", recorddir_base, script_bn) >> script;
    printf("\trecorddir=%s/%s\n", recorddir_base, script_bn) >> script;

    printf("else\n") >> script;

    recorddir_base = sprintf("%s/data/linux", autotest_home);
    printf("\trecordfile=%s/%s.output\n", recorddir_base, script_bn) >> script;
    printf("\trecorddir=%s/%s\n", recorddir_base, script_bn) >> script;
    
    printf("fi\n\n") >> script;

    printf("command_line='%s'\n\n", command_line) >> script;
    print(". ${AUTOTEST_HOME}/stress_test/util/run.sh") >> script;

    system("chmod +x " script);
}
