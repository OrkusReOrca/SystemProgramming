#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FIFO_NAME "/tmp/my_fifo"

int main(int argc, char *argv[]) {
  int res, open_mode = 0;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <[O_RDONLY, O_WRONLY, O_RDWR, O_NONBLOCK]>\n",
            *argv);

    exit(EXIT_FAILURE);
  }

  argv++;
  if (*argv) {
    if (strncmp(*argv, "O_RDONLY", 8) == 0)
      open_mode |= O_RDONLY;
    if (strncmp(*argv, "O_WRONLY", 8) == 0)
      open_mode |= O_WRONLY;
    if (strncmp(*argv, "O_RDWR", 6) == 0)
      open_mode |= O_RDWR;
    if (strncmp(*argv, "O_NONBLOCK", 10) == 0)
      open_mode |= O_NONBLOCK;
  }

  if (access(FIFO_NAME, F_OK) == -1) {
    res = mkfifo(FIFO_NAME, 0777);
    if (res) {
      fprintf(stderr, "Could not create fifo %s\n", FIFO_NAME);
      exit(EXIT_FAILURE);
    }
  }

  printf("Process %d opening FIFO\n", getpid());
  res = open(FIFO_NAME, open_mode);
  printf("Process %d result %d\n", getpid(), res);
  sleep(5);
  if (res != -1)
    (void) close(res);
  printf("Process %d finished\n", getpid());
  unlink(FIFO_NAME);
  exit(EXIT_SUCCESS);
}