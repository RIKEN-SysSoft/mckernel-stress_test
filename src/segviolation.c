#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <linux/futex.h>
#include <string.h>


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


void segViolation() {

	volatile int *addr = 0;
	*addr = 0;
}


void subjectTask(struct Thread* thread) {

	pthread_barrier_wait(&barrier);

	gettimeofday(&timeBeforeRead, NULL);
	printf("[INFO] pid: %d, tid: %d, file: %s, Setup time: %d ms\n", getpid(), syscall(SYS_gettid), __FILE__, LAPTIME_MS(timeBeforeFork, timeBeforeRead));

	printf("[INFO] pid: %d, tid: %d, file: %s, Trying to cause SEGV\n",  getpid(), syscall(SYS_gettid), __FILE__);

//	if (thread->tid == 0) {
		segViolation();
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

	printf("[INFO] pid: %d, tid: %d, file: %s, I am a subjectThread\n", getpid(), syscall(SYS_gettid), __FILE__);

	pthread_cleanup_push(subjectCleanup, arg);

	//sleep(random() % 5 + 1);
	//printf("[%d:%d] wake up\n", getpid(), thread->tid);

	pthread_barrier_wait(&barrier);

	subjectTask(thread);

	pthread_cleanup_pop(1);

	return NULL;
}


int examinerProcess(pid_t subject) {
	int ret;
	int status;

	printf("[INFO] pid: %d, file: %s, I am the examiner for %d.\n", getpid(), __FILE__, subject);

	if (waitpid(subject, &status, 0) < 0) {
		onError("waitpid fail");
	}

	if (WIFEXITED(status)) {
		int exit_status = WEXITSTATUS(status);
		printf("[FAIL] pid: %d, file: %s. Target process exited normally with return value %d\n", getpid(), __FILE__, exit_status);
		ret = exit_status == 0 ? -1 : exit_status;
		goto out;
	}

	if (WIFSIGNALED(status)) {
		int sig = WTERMSIG(status);
		if (sig == SIGSEGV) {
			printf("[PASS] pid: %d, file: %s, Target process is killed by the signal %d\n", getpid(), __FILE__, sig);
			ret = 0;
		} else {
			printf("[FAIL] pid: %d, file: %s, Target process is killed by the signal %d\n", getpid(), __FILE__, sig);
			ret = sig;
		}
	}

//	printf("TEST SUCCESSED IF YOU DID NOT SEE 'OVERRUN'\n");
//	printf("TEST FINISHED\n");
out:
	return ret;
}


int main(int _argc, char** _argv)
{
	int ret = 0;
	pid_t pid;
	
	argc = _argc;
	argv = _argv;

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

	printf("[INFO] pid: %d, file: %s, numthreads: %d\n", getpid(), __FILE__, numthreads);
	printf("[INFO] pid: %d, file: %s, nosignal: %d\n", getpid(), __FILE__, nosignal);

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
			ret = examinerProcess(pid);
		}
	}

	return ret;
}
