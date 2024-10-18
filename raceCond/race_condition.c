#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  pid_t pid;
  char *msg, *omsg, c;
  int n, flagfd, turn = 0;

  flagfd = open("flag", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  write(flagfd, &turn, 1);
  printf("fork program starting\n");
  pid = fork();
  switch (pid) {
  case -1:
    exit(1);
  case 0:
    msg = "Child";
    n = 35;
    break;
  default:
    msg = "Parent";
    n = 35;
    break;
  }

  setbuf(stdout, NULL); // set unbuffered
  
  omsg = msg;
  for (; n > 0; n--) {
    if (pid) {
      while (turn) {
        sleep(1);
        lseek(flagfd, 0, SEEK_SET);
        read(flagfd, &turn, 1);
      }
    } else {
      while (!turn) {
        sleep(1);
        lseek(flagfd, 0, SEEK_SET);
        read(flagfd, &turn, 1);
      }
    }
    while (c = *msg++)
      putc(c, stdout);

    msg = omsg;
    if (pid) {
      turn = 1;
      lseek(flagfd, 0, SEEK_SET);
      write(flagfd, &turn, 1);
    } else {
      turn = 0;
      lseek(flagfd, 0, SEEK_SET);
      write(flagfd, &turn, 1);
    }
  }
  return 0;
}