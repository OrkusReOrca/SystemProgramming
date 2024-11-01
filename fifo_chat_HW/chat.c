//Kunanon Thappawong 6581163
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define FIFO1 "fo1to2"
#define FIFO2 "fo2to1"
#define BUFFER_SIZE 1024

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

    // Create the FIFOs if they don't already exist
    mkfifo(FIFO1, 0666);
    mkfifo(FIFO2, 0666);

    int write_fd, read_fd;
    if (process == 1) {
        write_fd = open(FIFO1, O_RDWR);
        read_fd = open(FIFO2, O_RDWR);
    } else {
        write_fd = open(FIFO2, O_RDWR);
        read_fd = open(FIFO1, O_RDWR);
    }

    if (write_fd == -1 || read_fd == -1) {
        perror("Failed to open FIFOs");
        exit(EXIT_FAILURE);
    }

    printf("loading...\n");
    int connected = 0;
    char test_buf[1];
    while (!connected) {
        // Try a non-blocking read from the read FIFO to detect connection
        if (read(read_fd, test_buf, 0) >= 0) {
            connected = 1;
            printf("connected\n");
        } else {
            usleep(100000); // Sleep for 100ms to avoid high CPU usage
        }
    }

    // Fork two child processes: one for sending and one for receiving
    pid_t sender_pid = fork();

    if (sender_pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (sender_pid == 0) { // Sender process
        char buffer[BUFFER_SIZE];
        while (1) {
            printf("You: ");
            fgets(buffer, BUFFER_SIZE, stdin);

            // Remove newline from input
            buffer[strcspn(buffer, "\n")] = 0;

            write(write_fd, buffer, strlen(buffer) + 1);

            if (strncmp(buffer, "end chat", 8) == 0) {
                write(write_fd, "The other side have set to end the chat", 40);
                write(write_fd, "Type 'end chat' to end the chat", 31);
                printf("You've left the chat\nChat will end when they also done so\n");
                break;
            }
        }
        exit(0);
    }

    pid_t receiver_pid = fork();

    if (receiver_pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (receiver_pid == 0) { // Receiver process
        char buffer[BUFFER_SIZE];
        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            if (read(read_fd, buffer, BUFFER_SIZE) > 0) {
                if (strncmp(buffer, "end chat", 8) == 0) {
                    printf("\nThe other side shave left the chat\nType 'end chat' to leave and end the chat\n");
                    break;
                }
                printf("\nReceived: %s\n", buffer);
                printf("You: "); // Reprint "You: " after receiving a message
                fflush(stdout);
            }
            usleep(100000); // Small delay to avoid CPU overuse
        }
        exit(0);
    }

    // Wait for both sender and receiver child processes to finish
    waitpid(sender_pid, NULL, 0);
    waitpid(receiver_pid, NULL, 0);

    // Close FIFOs and clean up
    close(write_fd);
    close(read_fd);

    // Only process 1 should unlink the FIFOs
    if (process == 1) {
        unlink(FIFO1);
        unlink(FIFO2);
    }

    printf("Chat ended.\n");
    return 0;
}
