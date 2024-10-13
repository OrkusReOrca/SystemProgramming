// Kunanon 6581163
// Prime Finder wtih Thread for each range
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_NUM 50000
#define NUM_THREADS 5
#define RANGE 10000
#define MAX_PRIMES 10000

// Shared Array, store prime founded + amount
int primes[MAX_PRIMES];
int prime_count = 0;
pthread_mutex_t prime_mutex;

// Prime Checker
int is_prime(int num) {
  if (num <= 1)
    return 0;
  for (int i = 2; i * i <= num; i++) {
    if (num % i == 0)
      return 0;
  }
  return 1;
}

// Thread to do Prime Checker
void *compute_primes(void *arg) {
  int start = *(int *)arg;
  int end = start + RANGE;

  for (int i = start; i < end; i++) {
    if (is_prime(i)) {
      // Lock the mutex before modifying the shared array
      pthread_mutex_lock(&prime_mutex);
      primes[prime_count++] = i;
      pthread_mutex_unlock(&prime_mutex);
    }
  }
  pthread_exit(NULL);
}

int main() {
  pthread_t threads[NUM_THREADS];
  int start_range[NUM_THREADS];

  // Mutex start
  pthread_mutex_init(&prime_mutex, NULL);

  // Thread create
  for (int i = 0; i < NUM_THREADS; i++) {
    start_range[i] = i * RANGE + 1;
    pthread_create(&threads[i], NULL, compute_primes, &start_range[i]);
  }

  // Thread wait all done
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  // Print results
  printf("Prime numbers:\n");
  int lineCut = 0;
  for (int i = 0; i < prime_count; i++) {
    printf("%5d , ", primes[i]);
    lineCut++;
    if (lineCut >= 8) {
      printf("\n");
      lineCut = 0;
    }
  }

  // Mutex stop
  pthread_mutex_destroy(&prime_mutex);

  printf("\nPrime number search completed.\n");
  return 0;
}
