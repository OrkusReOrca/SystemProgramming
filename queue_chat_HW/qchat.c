// Kunanon Thappawong 6581163
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024
#define QUEUE_KEY 6581163

// Global variables to store child PIDs
pid_t sender_pid, receiver_pid;

// Message structure for System V message queue
struct message {
    long msg_type;
    char msg_text[BUFFER_SIZE];
};

void terminate_chat(int signum) {
    // Kill both sender and receiver processes
    if (sender_pid > 0) {
        kill(sender_pid, SIGTERM);
    }
    if (receiver_pid > 0) {
        kill(receiver_pid, SIGTERM);
    }
    printf("Chat terminated.\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <1 or 2>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int process = atoi(argv[1]);
    if (process != 1 && process != 2) {
        fprintf(stderr, "Argument must be 1 or 2\n");
        exit(EXIT_FAILURE);
    }

    // Create or get the message queue
    int msgid = msgget(QUEUE_KEY, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("Failed to create or get message queue");
        exit(EXIT_FAILURE);
    }

    printf("loading...\n");
    sleep(1); 
    printf("connected\n");

    // Set up signal handler for SIGTERM to terminate both processes
    signal(SIGTERM, terminate_chat);

    // Fork two child processes: one for sending and one for receiving
    sender_pid = fork();

    if (sender_pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (sender_pid == 0) { // Sender process
        struct message msg;
        msg.msg_type = (process == 1) ? 2 : 1;  // Set message type based on process
        while (1) {
            printf("You: ");
            fgets(msg.msg_text, BUFFER_SIZE, stdin);
            msg.msg_text[strcspn(msg.msg_text, "\n")] = 0;  // Remove newline character

            if (msgsnd(msgid, &msg, sizeof(msg.msg_text), 0) == -1) {
                perror("Failed to send message");
                break;
            }

            if (strncmp(msg.msg_text, "end chat", 8) == 0) {
                printf("You've left the chat\n");
                kill(getppid(), SIGTERM);  // Terminate the receiver process
                break;
            }
        }
        exit(0);
    }

    receiver_pid = fork();

    if (receiver_pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (receiver_pid == 0) { // Receiver process
        struct message msg;
        long msg_type_to_receive = (process == 1) ? 1 : 2;  // Set type to receive based on process
        while (1) {
            if (msgrcv(msgid, &msg, sizeof(msg.msg_text), msg_type_to_receive, 0) > 0) {
                if (strncmp(msg.msg_text, "end chat", 8) == 0) {
                    printf("\nOther side has LEFT the chat\n");
                    kill(getppid(), SIGTERM);  // Terminate the sender process
                    break;
                }
                printf("\nReceived: %s\n", msg.msg_text);
                printf("You: ");
                fflush(stdout);
            }
        }
        exit(0);
    }

    // Wait for both sender and receiver child processes to finish
    waitpid(sender_pid, NULL, 0);
    waitpid(receiver_pid, NULL, 0);

    // Only process 1 should delete the message queue
    if (process == 1) {
        msgctl(msgid, IPC_RMID, NULL);
    }

    printf("Chat ended.\n");
    return 0;
}
