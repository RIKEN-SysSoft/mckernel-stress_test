#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>

extern char** environ;
int argc;
char** argv;

void onError(char* message) {

	fprintf(stderr,  "%s: %s: %m", argv[0], message);
	exit(-1);
}


void examinerProcess(pid_t subject) {
	printf("[%d] I am the examiner for %d.\n", getpid(), subject);

	struct timespec req, rem;
	req.tv_sec = 0;
	req.tv_nsec = 500000000; // 500msec

	if (nanosleep(&req, &rem) < 0) {
		fprintf(stderr, "nanosleep is interrupted, but ignore\n");
	}

	kill(subject, SIGINT);

	int status;
	if (waitpid(subject, &status, 0) < 0) {
		onError("waitpid fail");
	}

	if (WIFEXITED(status)) {
		printf("The TEST process exited with return value %d\n", WEXITSTATUS(status));
		return;
	}

	if (WIFSIGNALED(status)) {
		printf("The TEST process is terminated by the signal %d\n", WTERMSIG(status));
	}

	printf("TEST SUCCESSED IF YOU DID NOT SEE 'OVERRUN'\n");
	printf("TEST FINISHED\n");
}


int main(int _argc, char** _argv)
{
	pid_t pid;

	argc = _argc;
	argv = _argv;
	
	if (argc < 2) {
		fprintf(stderr, "usage:\n"
			"%s command args...\n", argv[0]);
		return -1;
	}

	char** args = calloc(argc, sizeof(char*));
	int i;
	for (i = 0; i < argc - 1; i++) {
		args[i] = argv[i + 1];
	}
	args[i] = 0;

	pid = fork();
	if (pid < 0) {
		onError("fork");
	} else if (pid == 0) {
		execvpe(args[0], args, environ);
		onError("execve fail");
	} else {
		examinerProcess(pid);
	}

	return 0;
}
