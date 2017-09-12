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


#define MAXNUMTHREADS	256



int argc;
char** argv;
int numthreads = 2;
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


void subjectTask(struct Thread* self) {

	printf("[%d] START TEST\n", self->tid);

	struct timespec req, rem;
	req.tv_sec = 0;
	req.tv_nsec = 500000000; // 500msec

	if (nanosleep(&req, &rem) < 0) {
	  fprintf(stderr, "nanosleep is interrupted, but ignore\n");
	}

	if (self->tid % 2 == 0) {

		printf("[%d] Send SIGTERM to %d\n", self->tid, self->tid + 1);
		pthread_kill(thread[self->tid + 1].pthread, SIGTERM);

	} else {

		printf("[%d] Send SIGTERM to %d\n", self->tid, self->tid - 1);
		pthread_kill(thread[self->tid - 1].pthread, SIGTERM);

	}

	for(;;);

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
		if (WTERMSIG(status) == SIGTERM) {
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

	printf("DANGERTEST SIMULTANEOUSTERM\n");

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

	if ((numthreads % 2 != 0) || (numthreads < 2)) {
		fprintf(stderr, "%s: num threads must be a multiple of 2\n", argv[0]);
		exit(-1);
	}

	printf("NUMTHREADS: %d\n", numthreads);
	printf("NOSIGNAL: %d\n", nosignal);

	//	setup();

	gettimeofday(&timeBeforeFork, NULL);

	pid = fork();
	if (pid < 0) {
		onError("fork");
	} else if (pid == 0) {
		createThreads();
		joinThreads();
	} else {
		examinerProcess(pid);
	}

	return 0;
}
