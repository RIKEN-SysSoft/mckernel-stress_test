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

void onError(char* message) {

	fprintf(stderr,  "%s: %s: %m", argv[0], message);
	exit(-1);
}


void examinerProcess() {
	printf("[%d] I am the examiner.\n", getpid());

	struct timespec req, rem;

	req.tv_sec = waitmsec / 1000;
	req.tv_nsec = (waitmsec % 1000) * 1000000;

	if (nanosleep(&req, &rem) < 0) {
		fprintf(stderr, "nanosleep is interrupted, but ignore\n");
	}

	int i;

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
			printf("[%d] The TEST process exited with return value %d\n", pid, WEXITSTATUS(status));
		} else if (WIFSIGNALED(status)) {
			printf("[%d] The TEST process is terminated by the signal %d\n", pid, WTERMSIG(status));
		}

		printf("[%d] TEST SUCCESSED IF YOU DID NOT SEE 'OVERRUN'\n", pid);
		printf("[%d] TEST FINISHED\n", pid);
	}
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

	printf("DANGERTEST KILLIT\n");
	printf("NUMPROCESSES: %d\n", numprocesses);
	printf("WAITMSEC: %d\n", waitmsec);
	printf("NOSIGNAL: %d\n", nosignal);

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

	examinerProcess();

	return 0;
}
