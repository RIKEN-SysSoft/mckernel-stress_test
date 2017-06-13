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
#include <linux/futex.h>
#include <sys/syscall.h>

#if !defined(SINGLE) && !defined(MULTI)
#error CPP macro should be defined
#endif


#ifdef MULTI
#define NUMTHREADS 4
#else
#define NUMTHREADS 1
#endif

int argc;
char** argv;

//size_t bufferSize;
//char* buffer;

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


void* subjectThread(void*);

void createThreads() {

	if (pthread_barrier_init(&barrier, NULL, NUMTHREADS)) {
		onError("pthread_barrier_init fail");
	}

	int i;
	for (i = 1; i < NUMTHREADS; i++) {
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

	for (i = 1; i < NUMTHREADS; i++) {
		void* rval;
		if (pthread_join(thread[i].pthread, &rval)) {
			onError("pthread_join fail");
		}  
	}

	printf("Join done\n");
}


void stackCorruption() {

	int *addr;
	void *sp;

	__asm__("movq %%rsp,%0": "=r"(sp));
	addr = (int*) (sp + 8);
	*addr = 0; // overwrite return address
}


void subjectTask(struct Thread* thread) {

	pthread_barrier_wait(&barrier);

	gettimeofday(&timeBeforeRead, NULL);
	printf("[%d] setup: %d ms\n", thread->tid, LAPTIME_MS(timeBeforeFork, timeBeforeRead));

	printf("[%d] START TEST\n", thread->tid);

	if (thread->tid == 0) {
		stackCorruption();
	}

	for(;;);

	printf("%d TEST FAIL OVERRUN\n", thread->tid);

	gettimeofday(&timeAfterRead, NULL);

	for(;;);
//	exit(0);
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


void examinerProcess(pid_t subject) {
	printf("[%d] I am the examiner for %d.\n", getpid(), subject);

	struct timespec req, rem;
	req.tv_sec = 0;
	req.tv_nsec = 500000000; // 500msec

	if (nanosleep(&req, &rem) < 0) {
		fprintf(stderr, "nanosleep is interrupted, but ignore\n");
	}

	kill(subject, SIGTERM);
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

#if defined(SINGLE)
	printf("DANGERTEST STACKCORRUPTION SINGLE\n");
#elif defined(MULTI)
	printf("DANGERTEST STACKCORRUPTION MULTI\n");
#elif defined(NOSIGNAL)
	printf("DANGERTEST STACKCORRUPTION NOSIGNAL\n");
#endif

	//	setup();

	gettimeofday(&timeBeforeFork, NULL);

#ifdef NOSIGNAL
	createThreads();
	joinThreads();
#else
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
#endif

	return 0;
}
