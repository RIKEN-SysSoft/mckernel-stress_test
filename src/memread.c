#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

//#define BUFSIZE (1 * 1024 * 1024 * 1024)
//#define BUFSIZE (2 * 1024 * 1024)
#define BUFSIZE (0x800000)
//#define OFFSET (0x100000000)		// System RAM
#define OFFSET (0xff500000)	// reserved area

int main(int argc, char** argv)
{
  char* buffer;
  char* buffp;
  size_t size_to_read;
  ssize_t rval;
  int fd;
  char devicename[64];
  off_t offret;

  //snprintf(devicename, sizeof(devicename), "/proc/%d/mem", getpid());
  //snprintf(devicename, sizeof(devicename), "%s", argv[0]);
  snprintf(devicename, sizeof(devicename), "/dev/mem");

  buffer = malloc(BUFSIZE);
  if (buffer == NULL) {
    fprintf(stderr, "%s failed allocating buffer\n", argv[0]);
    return -1;
  }

  fd = open(devicename, O_RDONLY);
  if (fd < 0) {
    perror(argv[0]);
    return -1;
  }

  offret = lseek(fd, OFFSET, SEEK_SET);
  if (offret < 0) {
    perror(argv[0]);
    return -1;
  }

  size_to_read = BUFSIZE;
  buffp = buffer;
  while (size_to_read > 0) {
    fprintf(stderr, "%s try to read %lld bytes\n", argv[0], size_to_read);
    rval = read(fd, buffp, size_to_read);
    if (rval == 0) {
      break;
    }
    if (rval < 0) {
      perror(argv[0]);
      return -1;
    }
    size_to_read -= rval;
    buffp += rval;
  }

  fprintf(stderr, "%s read %lld bytes\n", argv[0], buffp - buffer);

  return 0;
}
