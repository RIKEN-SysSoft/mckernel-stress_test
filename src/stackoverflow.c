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
#include <string.h>
#include <alloca.h>


#define MAXNUMTHREADS	256

int argc;
char** argv;
int numthreads = 1;
int nosignal = 0;

struct timeval timeBeforeFork;
struct timeval timeBeforeRead;
struct timeval timeAfterRead;


#define LAPTIME_MS(start, stop) ((stop.tv_sec - start.tv_sec) * 1000 + (stop.tv_usec - start.tv_usec) / 1000)

struct Thread {
	int tid;
	pthread_t pthread;
} thread[MAXNUMTHREADS];

pthread_barrier_t barrier;

void onError(char* message) {

	fprintf(stderr,  "%s: %s: %m", argv[0], message);
	exit(-1);
}


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


void stackOverflow(int order) {
	void *tmp, *cur;
	size_t size = 1UL << order;
	long pgsize = sysconf(_SC_PAGESIZE);

	printf("allocating %d bytes\n", size);
	tmp = alloca(size);
	for (cur = tmp; cur < tmp + size; cur += pgsize) {
	     *((int *)cur) = 0x12345678;
	}

	stackOverflow(order >= 30 ? 30 : order + 1);
}

void subjectTask(struct Thread* thread) {

	pthread_barrier_wait(&barrier);

	gettimeofday(&timeBeforeRead, NULL);
	printf("[%d] setup: %d ms\n", thread->tid, LAPTIME_MS(timeBeforeFork, timeBeforeRead));

	printf("[%d] START TEST\n", thread->tid);

//	if (thread->tid == 0) {
	stackOverflow(16);
//	}

	printf("%d TEST FAIL OVERRUN\n", thread->tid);

	for(;;);

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

	int status;
	if (waitpid(subject, &status, 0) < 0) {
		onError("waitpid fail");
	}

	if (WIFEXITED(status)) {
		printf("The TEST process unexpectedly exited with return value %d\n", WEXITSTATUS(status));
		printf("TEST FAILED\n");
		if (WEXITSTATUS(status) == 0) {
			exit(-1);
		} else {
			exit(WEXITSTATUS(status));
		}
		return;
	}

	if (WIFSIGNALED(status)) {
		printf("The TEST process is terminated by the signal %d\n", WTERMSIG(status));
		if (WTERMSIG(status) == SIGSEGV) {
			printf("TEST SUCCESSED\n");
		} else {
			printf("TEST FAILED\n");
			exit(WTERMSIG(status));
		}
	}

//	printf("TEST SUCCESSED IF YOU DID NOT SEE 'OVERRUN'\n");
//	printf("TEST FINISHED\n");
}


int main(int _argc, char** _argv)
{
	pid_t pid;
	
	argc = _argc;
	argv = _argv;

	printf("DANGERTEST STACKOVERFLOW\n");

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
		fprintf(stderr, "%s: argument error\n"
			"Usage:\n"
			"\t-nt <num threads>\n"
			"\t-nosignal\n", argv[0]);
		exit(-1);
	}

	if (numthreads < 1 || numthreads > MAXNUMTHREADS) {
		fprintf(stderr, "%s: invalid num threads\n", argv[0]);
		exit(-1);
	}

	printf("NUMTHREADS: %d\n", numthreads);
	printf("NOSIGNAL: %d\n", nosignal);

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
