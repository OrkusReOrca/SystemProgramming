// Kunanon Thappawong 6581163
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define SHM_KEY 21930

// Global variables to store child PIDs
pid_t sender_pid, receiver_pid;

// Shared memory structure
struct shared_memory {
  sem_t sem;                  // Semaphore to synchronize access
  int msg_from_1;             // Flag for process 1's message status
  int msg_from_2;             // Flag for process 2's message status
  char msg_text[BUFFER_SIZE]; // Shared message text
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

  // Create or get shared memory
  int shmid = shmget(SHM_KEY, sizeof(struct shared_memory), 0666 | IPC_CREAT);
  if (shmid == -1) {
    perror("Failed to create or get shared memory");
    exit(EXIT_FAILURE);
  }

  // Attach the shared memory segment
  struct shared_memory *shm = (struct shared_memory *)shmat(shmid, NULL, 0);
  if (shm == (void *)-1) {
    perror("Failed to attach shared memory");
    exit(EXIT_FAILURE);
  }

  // Initialize semaphore and flags
  if (process == 1) {            // Only one process should initialize
    sem_init(&(shm->sem), 1, 1); // Semaphore for shared memory access
    shm->msg_from_1 = 0;
    shm->msg_from_2 = 0;
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
    while (1) {
      // Get input message
      printf("You: ");
      fgets(shm->msg_text, BUFFER_SIZE, stdin);
      shm->msg_text[strcspn(shm->msg_text, "\n")] =
          0; // Remove newline character

      // Lock semaphore before writing
      sem_wait(&(shm->sem));

      // Set the appropriate flag based on the process number
      if (process == 1) {
        shm->msg_from_1 = 1; // Process 1 has written a message
      } else {
        shm->msg_from_2 = 1; // Process 2 has written a message
      }

      // Check if user wants to end chat
      if (strncmp(shm->msg_text, "end chat", 8) == 0) {
        printf("You've left the chat\n");
        kill(getppid(), SIGTERM); // Terminate the receiver process
        sem_post(&(shm->sem));    // Unlock semaphore
        break;
      }

      sem_post(&(shm->sem)); // Release semaphore after writing
      usleep(500000);        // Small delay to prevent CPU overuse
    }
    exit(0);
  }

  receiver_pid = fork();

  if (receiver_pid < 0) {
    perror("Fork failed");
    exit(EXIT_FAILURE);
  } else if (receiver_pid == 0) { // Receiver process
    while (1) {
      // Lock semaphore to read the message
      sem_wait(&(shm->sem));

      // Check for a new message from the other process
      if ((process == 1 && shm->msg_from_2 == 1) ||
          (process == 2 && shm->msg_from_1 == 1)) {
        // Check if the other process has ended the chat
        if (strncmp(shm->msg_text, "end chat", 8) == 0) {
          printf("\nOther side has LEFT the chat\n");
          kill(getppid(), SIGTERM); // Terminate the sender process
          sem_post(&(shm->sem));    // Unlock semaphore
          break;
        }

        // Print the received message
        printf("\nReceived: %s\n", shm->msg_text);
        printf("You: ");
        fflush(stdout);

        // Reset the flag for the received message
        if (process == 1) {
          shm->msg_from_2 = 0;
        } else {
          shm->msg_from_1 = 0;
        }
      }

      sem_post(&(shm->sem)); // Release semaphore after reading
      usleep(500000);        // Small delay to prevent CPU overuse
    }
    exit(0);
  }

  // Wait for both sender and receiver child processes to finish
  waitpid(sender_pid, NULL, 0);
  waitpid(receiver_pid, NULL, 0);

  // Cleanup
  if (process == 1) {              // Only process 1 should perform cleanup
    sem_destroy(&(shm->sem));      // Destroy semaphore
    shmctl(shmid, IPC_RMID, NULL); // Remove shared memory segment
  }

  printf("Chat ended.\n");
  return 0;
}
