
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    pid_t pid; char *msg, c; int n;

    printf("fork program starting\n");
    pid = fork();
    switch(pid) {
        case -1:
            exit(1);
        case 0:
            msg = "C"; n = 35;
            break;
        default:
            msg = "P";
            n = 35;
        break;
    }

    setbuf(stdout, NULL); // set unbuffered
    for (; n>0; n--) {
        while (c = *msg++) {
            putc(c, stdout);
            sleep(1);
        }
    }
    if (pid) {
        int stat_val; pid_t child_pid;
        child_pid = wait(&stat_val);
        printf("Child has nished: PID = %d\n", child_pid);
        if (WIFEXITED(stat_val))
            printf("Child exited with code %d\n", WEXITSTATUS(stat_val));
        else
            printf("Child terminated abnormally\n");
    }
    exit(0);
}//sdasd