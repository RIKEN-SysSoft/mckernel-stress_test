#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>

#define BUFFSIZE (2LL * 1024LL * 1024LL * 1024LL)
//#define BUFFSIZE (1024LL * 1024LL)

int argc;
char** argv;

size_t bufferSize;
char* buffer;

struct timeval timeBeforeFork;
struct timeval timeBeforeRead;
struct timeval timeAfterRead;

#define LAPTIME_MS(start, stop) ((stop.tv_sec - start.tv_sec) * 1000 + (stop.tv_usec - start.tv_usec) / 1000)

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

void subjectProcess() {

	printf("[%d] I am a subject.\n", getpid());

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

	exit(0);
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
		subjectProcess();
	} else {
		examinerProcess(pid);
	}

	return 0;
}
