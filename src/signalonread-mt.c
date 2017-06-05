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

#define BUFFSIZE (2LL * 1024LL * 1024LL * 1024LL)
//#define BUFFSIZE (1024LL * 1024LL)

#define NUMTHREADS 4

int argc;
char** argv;

size_t bufferSize;
char* buffer;

struct timeval timeBeforeFork;
struct timeval timeBeforeRead;
struct timeval timeAfterRead;


#define LAPTIME_MS(start, stop) ((stop.tv_sec - start.tv_sec) * 1000 + (stop.tv_usec - start.tv_usec) / 1000)

struct Thread {
	int tid;
	pthread_t pthread;
} thread[NUMTHREADS];

pthread_barrier_t barrier;

void onError(char* message) {

	fprintf(stderr,  "%s: %s: %m", argv[0], message);
	exit(-1);
}

void setup() {

	bufferSize = BUFFSIZE;
	buffer = malloc(BUFFSIZE);
	if (buffer == NULL) {
		onError("malloc fail");
	}
//	printf("buffer: %p\n", buffer);
}


void* subjectThread(void*);

void createThreads() {

	if (pthread_barrier_init(&barrier, NULL, NUMTHREADS)) {
		onError("pthread_barrier_init fail");
	}

	int i;
	for (i = 0; i < NUMTHREADS; i++) {
		int rval;
		thread[i].tid = i;
		rval = pthread_create(&thread[i].pthread, NULL, subjectThread, &thread[i]);
		if (rval) {
			onError("pthread_create fail");
		}
	}

}


void joinThreads() {

	int i;

	for (i = 0; i < NUMTHREADS; i++) {
		void* rval;
		if (pthread_join(thread[i].pthread, &rval)) {
			onError("pthread_join fail");
		}  
	}

	printf("Join done\n");
}


void subjectTask() {
	int fd;
	fd = open("../tmp/largefile.dat", O_RDONLY);
	if (fd < 0) {
		onError("open fail");
	}

	gettimeofday(&timeBeforeRead, NULL);
	printf("setup: %d ms\n", LAPTIME_MS(timeBeforeFork, timeBeforeRead));

	char* buffp;
	size_t size_to_read;
	ssize_t rval;
	buffp = buffer;
	size_to_read = BUFFSIZE;
	while (size_to_read > 0) {
		fprintf(stderr, "%s try to read %lld bytes. buffp=%p\n", argv[0], size_to_read, buffp);
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

	gettimeofday(&timeAfterRead, NULL);

	fprintf(stderr, "%s read %lld bytes\n", argv[0], buffp - buffer);
	printf("read: %d ms\n", LAPTIME_MS(timeBeforeRead, timeAfterRead));

	for(;;);
	exit(0);
}


void subjectProcess() {

	printf("[%d] I am a subject.\n", getpid());

	subjectTask();
}


void* subjectThread(void* arg) {

	struct Thread* thread = (struct Thread*)arg;

	printf("[%d] I am a subjectThread %d, %x %x\n", getpid(), thread->tid, thread->pthread, pthread_self());


	sleep(random() % 5 + 1);

	printf("[%d:%d] wake up\n", getpid(), thread->tid);

	pthread_barrier_wait(&barrier);

	printf("[%d:%d] barrier ok\n", getpid(), thread->tid);

	if (thread->tid != 0) {
		for(;;);
	}

	subjectTask();

	return NULL;
}


void examinerProcess(pid_t subject) {
	printf("[%d] I am the examiner for %d.\n", getpid(), subject);

	struct timespec req, rem;
	req.tv_sec = 8;
	req.tv_nsec = 500000000; // 500msec

	if (nanosleep(&req, &rem) < 0) {
		fprintf(stderr, "nanosleep is interrupted, but ignore\n");
	}

	kill(subject, SIGINT);
	if (waitpid(subject, NULL, 0) < 0) {
		onError("waitpid fail");
	}

	printf("FINISH\n");
}


int main(int _argc, char** _argv)
{
	pid_t pid;
	
	argc = _argc;
	argv = _argv;

	setup();

	gettimeofday(&timeBeforeFork, NULL);
	pid = fork();
	if (pid < 0) {
		onError("fork");
	} else if (pid == 0) {
//		subjectProcess();
		createThreads();
		joinThreads();
	} else {
		examinerProcess(pid);
	}

	return 0;
}
