#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <string.h>

#define MAXNUMPROCESSES 256

extern char** environ;
int argc;
char** argv;
int numprocesses = 1;
int waitmsec = 500; // 500msec
int nosignal = 0;

pid_t subjectpid[MAXNUMPROCESSES];
int exit_statuses[MAXNUMPROCESSES];

void onError(char* message) {

	fprintf(stderr,  "%s: %s: %m", argv[0], message);
	exit(-1);
}


int examinerProcess() {
	int ret;
	int i;
	struct timespec req, rem;

	printf("[INFO] pid: %d, file: %s, I am the examiner for ", getpid(), __FILE__);
	for (i = 0; i < numprocesses; i++) {
		if (i > 0) {
			printf(",");
		}
		printf("%d", subjectpid[i]);
	}
	printf("\n");


	req.tv_sec = waitmsec / 1000;
	req.tv_nsec = (waitmsec % 1000) * 1000000;

	if (nanosleep(&req, &rem) < 0) {
		fprintf(stderr, "nanosleep is interrupted, but ignore\n");
	}

	if (!nosignal) {
		for (i = 0; i < numprocesses; i++) {
			if (kill(subjectpid[i], SIGINT) < 0) {
				printf("[%d] TEST EXITED ALREADY\n", subjectpid[i]);
				subjectpid[i] = 0;
			}
		}
	}

	for (i = 0; i < numprocesses; i++) {
		int status;
		pid_t pid = subjectpid[i];

		if (pid == 0) continue;

		if (waitpid(pid, &status, 0) < 0) {
			onError("waitpid fail");
		}
		
		if (WIFEXITED(status)) {
			int exit_status = WEXITSTATUS(status);
			if (WEXITSTATUS(status) == 0) {
				printf("[PASS] pid: %d, file: %s, Target process %d exited normally with return value %d\n", getpid(), __FILE__, pid, exit_status);
				exit_statuses[i] = 0;
			} else {
				printf("[FAIL] pid: %d, file: %s. Target process %d exited normally with return value %d\n", getpid(), __FILE__, pid, exit_status);
				exit_statuses[i] = exit_status;
			}
		} else if (WIFSIGNALED(status)) {
			int sig = WTERMSIG(status);
			if (sig == SIGINT) {
				printf("[PASS] pid: %d, file: %s, Target process %d is killed by the signal %d\n", getpid(), __FILE__, pid, sig);
				exit_statuses[i] = 0;
			} else {
				printf("[FAIL] pid: %d, file: %s, Target process %d is killed by the signal %d\n", getpid(), __FILE__, pid, sig);
				exit_statuses[i] = sig;
			}
		}
	}

	/* Only report the first failure */
	ret = 0;
	for (i = 0; i < numprocesses; i++) {
		if (exit_statuses[i] != 0) {
			ret = exit_statuses[i];
			goto out;
		}
	}
out:
	return ret;
}


int main(int _argc, char** _argv)
{
	int ret;
	pid_t pid;

	argc = _argc;
	argv = _argv;
	
	if (argc < 2) {
		fprintf(stderr, "usage:\n"
			"%s command args...\n", argv[0]);
		return -1;
	}

	int i = 1;
	while (*argv[i] == '-') {
		if (strcmp("-np", argv[i]) == 0) {
			i++;
			if (i < argc) {
				numprocesses = atoi(argv[i++]);
				continue;
			}
			fprintf(stderr, "%s: num processes required\n", argv[0]);
			return -1;
		}
		if (strcmp("-t", argv[i]) == 0) {
			i++;
			if (i < argc) {
				waitmsec = atoi(argv[i++]);
				printf("waitmsec %d\n", waitmsec);
				continue;
			}
			fprintf(stderr, "%s: timer value (msec) required\n", argv[0]);
			return -1;
		}
		if (strcmp("-nosignal", argv[i]) == 0) {
			i++;
			nosignal = 1;
			continue;
		}
		fprintf(stderr, "%s: invalid option\n", argv[0]);
		return -1;
	}

	if (numprocesses < 1 || numprocesses > MAXNUMPROCESSES) {
		fprintf(stderr, "%s: invalid num processes\n", argv[0]);
		return -1;
	}

	if (waitmsec < 0) {
		fprintf(stderr, "%s: invalid timer value\n", argv[0]);
		return -1;
	}

	printf("[INFO] pid: %d, file: %s, numprocesses: %d\n", getpid(), __FILE__, numprocesses);
	printf("[INFO] pid: %d, file: %s, waitmsec: %d\n", getpid(), __FILE__, waitmsec);
	printf("[INFO] pid: %d, file: %s, nosignal: %d\n", getpid(), __FILE__, nosignal);

	char** args = calloc(argc, sizeof(char*));
	int j = 0;
	for (; i < argc; i++) {
		args[j] = argv[i];
		j++;
	}
	args[j] = 0;

	for (i = 0; i < numprocesses; i++) {
		pid = fork();
		if (pid < 0) {
			onError("fork");
		} else if (pid == 0) {
			execvpe(args[0], args, environ);
			onError("execve fail");
		}
		subjectpid[i] = pid;
	}

	ret = examinerProcess();

	return ret;
}
