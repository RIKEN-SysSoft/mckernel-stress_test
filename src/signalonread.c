#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <string.h>
#include <libgen.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/mman.h>

static sem_t *sem = MAP_FAILED;

#define MAXBUFFSIZE (2LL * 1024LL * 1024LL * 1024LL)
#define MAXNUMTHREADS	256
#define DEFAULTTIMETOWAIT 500

int argc;
char** argv;
int nosignal = 0;
size_t buffsize = MAXBUFFSIZE;
int timetowait = DEFAULTTIMETOWAIT;

struct timeval timeBeforeFork;
struct timeval timeBeforeRead;
struct timeval timeAfterRead;

#define LAPTIME_MS(start, stop) ((stop.tv_sec - start.tv_sec) * 1000 + (stop.tv_usec - start.tv_usec) / 1000)

#define onError(fmt, args...) \
do { \
	fprintf(stderr, "[INTERR] " fmt "\n", ##args);	\
	exit(-1); \
} while(0)

void reader(void) {
	char* buffer = malloc(buffsize);
	if (buffer == NULL) {
		onError("malloc fail");
	}
	pid_t pid = getpid();

	int fd;
	char filename[1024];
	snprintf(filename, sizeof(filename), "./largefile.dat", dirname(argv[0]));

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		onError("open fail");
	}

	sem_post(sem);

	int semval;
	sem_getvalue(sem, &semval);
	fprintf(stderr, "[INFO] child %d: cpu: %d, sem value: %d\n",
		pid, sched_getcpu(), semval);

	gettimeofday(&timeBeforeRead, NULL);
	fprintf(stderr, "[INFO] child %d: fork and pthread_create took %d ms\n",
	       pid, LAPTIME_MS(timeBeforeFork, timeBeforeRead));

	char* buffp;
	size_t size_to_read;
	ssize_t rval;
	buffp = buffer;
	size_to_read = buffsize;
	while (size_to_read > 0) {
		rval = read(fd, buffp, 1UL << 30);
		if (rval == 0) {
			break;
		}
		if (rval < 0) {
			if (errno != EINTR) {
				onError("read failed with %d", errno);
			}
			fprintf(stderr, "[INFO] child %d: interrupted\n", pid);
			continue;
		}
		size_to_read -= rval;
		buffp += rval;
		fprintf(stdout, "[ NG ] child %d: read: %lld bytes\n",
			pid, rval);
	}

	gettimeofday(&timeAfterRead, NULL);

	fprintf(stderr, "[INFO] child %d: total: %lld bytes\n", pid, buffp - buffer);
	fprintf(stderr, "[INFO] child %d: took %d ms\n", pid, LAPTIME_MS(timeBeforeRead, timeAfterRead));
	fprintf(stderr, "[ NG ] child %d: expected: killed, actual: read done\n", pid);
	return;
}


void tvsub(struct timeval *tv1, struct timeval *tv2)
{
	tv1->tv_sec -= tv2->tv_sec;
	tv1->tv_usec -= tv2->tv_usec;
	if (tv1->tv_usec < 0) {
		tv1->tv_usec += 1000000;
		tv1->tv_sec--;
	}
}

void killer(pid_t subject) {

	struct timespec req, rem;
	struct timeval tv_start;
	struct timeval tv_end;
	struct timeval tv_wk;

	req.tv_sec = timetowait / 1000;
	req.tv_nsec = (timetowait % 1000) * 1000000;
	tv_wk.tv_sec = req.tv_sec;
	tv_wk.tv_usec = (timetowait %1000) * 1000;

	/* wait until fork() completes */
	fprintf(stderr, "[INFO] parent: cpu: %d, wait until fork() completes\n", sched_getcpu());
	sem_wait(sem);
	fprintf(stderr, "[INFO] parent: cpu: %d, woken up\n", sched_getcpu());

	gettimeofday(&tv_start, NULL);

	fprintf(stderr, "[INFO] parent: wait for a while before sending singal...\n");
	if (nanosleep(&req, &rem) < 0) {
		fprintf(stderr, "nanosleep is interrupted, but ignore\n");
	}

	fprintf(stderr, "[INFO] parent: sending SIGTERM to %d\n", subject);
	if (kill(subject, SIGTERM) < 0) {
		fprintf(stderr, "[ NG ] child process not found\n");
		exit (-1);
	}

	int status;
	if (waitpid(subject, &status, 0) < 0) {
		onError("waitpid failed with %d", errno);
	}
	gettimeofday(&tv_end, NULL);

	tvsub(&tv_end, &tv_start);
	tvsub(&tv_end, &tv_wk);
	if (tv_end.tv_sec >= 3) {
		fprintf(stderr, "[ NG ] response time (%f) >= 3 sec\n",
			tv_end.tv_sec + (double)tv_end.tv_usec / 1000000);
		return;
	} else {
		fprintf(stderr, "[ OK ] response time (%f) < 3 sec\n",
			tv_end.tv_sec + (double)tv_end.tv_usec / 1000000);
	}

	if (WIFEXITED(status)) {
		fprintf(stderr, "[ NG ] child process exited by itself with return value of %d\n",
		       WEXITSTATUS(status));
		if (WEXITSTATUS(status) == 0) {
			exit(-1);
		} else {
			exit(WEXITSTATUS(status));
		}
		return;
	}

	if (WIFSIGNALED(status)) {
		int sig = WTERMSIG(status);

		if (sig == SIGTERM) {
			fprintf(stderr, "[ OK ] child process is killed by the expected signal (%d)\n",
			       sig);
		} else {
			fprintf(stderr, "[ NG ] child process is killed by an unexpected signal (%d)\n",
			       sig);
			exit(sig);
		}
	}
}


int main(int _argc, char** _argv)
{
	pid_t pid;
	
	argc = _argc;
	argv = _argv;

	fprintf(stderr, "DANGERTEST SIGNALONREAD\n");

	int i;
	for (i = 1; i < argc; i++) {
		if (strcmp("-nosignal", argv[i]) == 0) {
			nosignal = 1;
			continue;
		}
		if (strcmp("-t", argv[i]) == 0) {
			i++;
			if (i < argc) {
				timetowait = atoi(argv[i]);
				continue;
			}
		}
		if (strcmp("-s", argv[i]) == 0) {
			i++;
			if (i < argc) {
				buffsize = atoll(argv[i]);
				continue;
			}
		}
		fprintf(stderr, "%s: argument error\n"
			"Usage:\n"
			"\t-nt <num threads>\n"
			"\t-nosignal\n"
			"\t-t <time to wait (msec)>\n"
			"\t-s <size to receive (bytes)>\n",
			argv[0]);
		exit(-1);
	}

	fprintf(stderr, "NOSIGNAL: %d\n", nosignal);
	fprintf(stderr, "TIMETOWAIT: %d msec\n", timetowait);
	fprintf(stderr, "BUFFSIZE: %lld\n", buffsize);

	sem = (sem_t *)mmap(NULL, sizeof(sem_t),
			     PROT_READ | PROT_WRITE,
			     MAP_SHARED | MAP_ANONYMOUS,
			     -1, 0);
	if (sem == MAP_FAILED) {
		onError("allocating sem");
	}

	sem_init(sem, 1, 0);

	gettimeofday(&timeBeforeFork, NULL);

	if (nosignal) {
		reader();
	} else {
		pid = fork();
		if (pid < 0) {
			onError("fork");
		} else if (pid == 0) {
			reader();
		} else {
			killer(pid);
		}
	}

	return 0;
}
