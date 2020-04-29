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


#define MAXBUFFSIZE (2LL * 1024LL * 1024LL * 1024LL)
#define MAXNUMTHREADS	256
#define DEFAULTTIMETOWAIT 500

int argc;
char** argv;
int numthreads = 1;
int nosignal = 0;
size_t buffsize = MAXBUFFSIZE;
int timetowait = DEFAULTTIMETOWAIT;

struct timeval timeBeforeFork;
struct timeval timeBeforeRead;
struct timeval timeAfterRead;


#define LAPTIME_MS(start, stop) ((stop.tv_sec - start.tv_sec) * 1000 + (stop.tv_usec - start.tv_usec) / 1000)

struct Thread {
	int tid;
	pthread_t pthread;
} thread[MAXNUMTHREADS];

pthread_barrier_t barrier;

#define onError(fmt, args...) \
do { \
	fprintf(stderr, "[ NG ] " fmt "\n", ##args); \
	exit(-1); \
} while(0)


void* subjectThread(void*);

void createThreads() {

	if (pthread_barrier_init(&barrier, NULL, numthreads)) {
		onError("pthread_barrier_init fail");
	}

	int i;
	for (i = 1; i < numthreads; i++) {
		int rval;
		thread[i].tid = i;
		rval = pthread_create(&thread[i].pthread, NULL, subjectThread, &thread[i]);
		if (rval) {
			onError("pthread_create fail");
		}
	}

	thread[0].tid = 0;
	thread[0].pthread = pthread_self();
	subjectThread(&thread[0]);
}


void joinThreads() {

	int i;

	for (i = 1; i < numthreads; i++) {
		void* rval;
		if (pthread_join(thread[i].pthread, &rval)) {
			onError("pthread_join fail");
		}  
	}

	printf("Join done\n");
}


void subjectTask(struct Thread* thread) {

	char* buffer = malloc(buffsize);
	if (buffer == NULL) {
		onError("malloc fail");
	}

	int fd;
	char filename[1024];
	snprintf(filename, sizeof(filename), "./largefile.dat", dirname(argv[0]));

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		onError("open fail");
	}

	pthread_barrier_wait(&barrier);

	gettimeofday(&timeBeforeRead, NULL);
	printf("[%d] setup took %d ms (should be less than %d ms)\n",
	       thread->tid, LAPTIME_MS(timeBeforeFork, timeBeforeRead),
	       timetowait);

	char* buffp;
	size_t size_to_read;
	ssize_t rval;
	buffp = buffer;
	size_to_read = buffsize;
	while (size_to_read > 0) {
		fprintf(stderr, "[%d] reading %lld bytes...\n",
			thread->tid, size_to_read, buffp);
		rval = read(fd, buffp, size_to_read);
		if (rval == 0) {
			break;
		}
		if (rval < 0) {
			onError("read fail");
		}
		size_to_read -= rval;
		buffp += rval;
	}

	printf("[ NG ] %d-th thread didn't received a signal while reading\n",
	       thread->tid);

	gettimeofday(&timeAfterRead, NULL);

	fprintf(stderr, "[%d] %s read %lld bytes\n", thread->tid, argv[0], buffp - buffer);
	printf("[%d] read: %d ms\n", thread->tid, LAPTIME_MS(timeBeforeRead, timeAfterRead));

//	for(;;);
	exit(-1);
}


void subjectProcess() {

	printf("[%d] I am a subject.\n", getpid());

//	Subjecttask();
}


void subjectCleanup(void* arg) {

	struct Thread* thread = (struct Thread*) arg;
	printf("[%d] cleanup\n", thread->tid);
}


void* subjectThread(void* arg) {

	struct Thread* thread = (struct Thread*)arg;

	printf("[%d] I am a subjectThread %d, %x %x\n", getpid(), thread->tid, thread->pthread, pthread_self());

	pthread_cleanup_push(subjectCleanup, arg);

	//sleep(random() % 5 + 1);
	//printf("[%d:%d] wake up\n", getpid(), thread->tid);

	pthread_barrier_wait(&barrier);

	subjectTask(thread);

	pthread_cleanup_pop(1);

	return NULL;
}

void
tvsub(struct timeval *tv1, struct timeval *tv2)
{
	tv1->tv_sec -= tv2->tv_sec;
	tv1->tv_usec -= tv2->tv_usec;
	if (tv1->tv_usec < 0) {
		tv1->tv_usec += 1000000;
		tv1->tv_sec--;
	}
}

void examinerProcess(pid_t subject) {
	printf("[%d] I am the examiner for %d.\n", getpid(), subject);

	struct timespec req, rem;
	struct timeval tv_start;
	struct timeval tv_end;
	struct timeval tv_wk;

	req.tv_sec = timetowait / 1000;
	req.tv_nsec = (timetowait % 1000) * 1000000;
	tv_wk.tv_sec = req.tv_sec;
	tv_wk.tv_usec = (timetowait %1000) * 1000;

	gettimeofday(&tv_start, NULL);
	if (nanosleep(&req, &rem) < 0) {
		fprintf(stderr, "nanosleep is interrupted, but ignore\n");
	}

	if (kill(subject, SIGTERM) < 0) {
		printf("[ NG ] child process not found\n");
		exit (-1);
	}

	int status;
	if (waitpid(subject, &status, 0) < 0) {
		onError("waitpid fail");
	}
	gettimeofday(&tv_end, NULL);

	tvsub(&tv_end, &tv_start);
	tvsub(&tv_end, &tv_wk);
	if (tv_end.tv_sec >= 3) {
		onError("[ NG ] response time (%f) is greater or equal to 3 second",
			tv_end.tv_sec + (double)tv_end.tv_usec / 1000000);
	}

	if (WIFEXITED(status)) {
		printf("[ NG ] child process exited by itself with return value of %d\n",
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
			printf("[ OK ] child process is killed by the expected signal (%d)\n",
			       sig);
		} else {
			printf("[ NG ] child process is killed by an unexpected signal (%d)\n",
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

	printf("DANGERTEST SIGNALONREAD\n");

	int i;
	for (i = 1; i < argc; i++) {
		if (strcmp("-nt", argv[i]) == 0) {
			i++;
			if (i < argc) {
				numthreads = atoi(argv[i]);
				continue;
			}
			fprintf(stderr, "%s: num threads required\n", argv[0]);
			exit(-1);
		}
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

	if (numthreads < 1 || numthreads > MAXNUMTHREADS) {
		fprintf(stderr, "%s: invalid num threads\n", argv[0]);
		exit(-1);
	}

	printf("NUMTHREADS: %d\n", numthreads);
	printf("NOSIGNAL: %d\n", nosignal);
	printf("TIMETOWAIT: %d msec\n", timetowait);
	printf("BUFFSIZE: %lld\n", buffsize);

	//	setup();

	gettimeofday(&timeBeforeFork, NULL);

	if (nosignal) {
		createThreads();
		joinThreads();
	} else {
		pid = fork();
		if (pid < 0) {
			onError("fork");
		} else if (pid == 0) {
			createThreads();
			joinThreads();
		} else {
			examinerProcess(pid);
		}
	}

	return 0;
}