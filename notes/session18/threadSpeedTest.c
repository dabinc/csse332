#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>

#define WORK_SIZE 20000

// Many thanks to Micah for the initial starting code (which I have modified
// significantly)

/* This code runs some long running calculations using various types
   of parallelism.  You can adjust the work size above to change the
   amount of work done.

   First play a bit with the code and get a feel for the speedup with different
   approaches.

   Then, modify PART 3 so that it runs a number of threads equal to THREAD_COUNT
   rather than just 2.
   */

#define THREAD_COUNT 5

float* dest;

void output_time_difference(char* name, struct timeval* start,
                            struct timeval* end) {
  long secs_used =
    (end->tv_sec - start->tv_sec);  // avoid overflow by subtracting first
  long usecs_used = (end->tv_usec - start->tv_usec);
  double secs = secs_used + (double)usecs_used / 1000000;
  printf("%s took %f seconds\n", name, secs);
}

// naive exponentiation is useful because it requires a lot of compute
int power(int a, int b) {
  int i;
  int r = a;
  for (i = 1; i < b; i++) r = r * a;
  return r;
}

void* threadFun(void* startingInt) {
  int start = *((int*)startingInt);
  for (int i = start; i < WORK_SIZE; i += 2) {
    dest[i] = power(i, i);
  }
}

int main(int argc, char** argv) {
  struct timeval start, end;
  int i;
  dest = (float*)malloc(sizeof(float) * WORK_SIZE);

  // PART 1: ------------------------no paralellism

  gettimeofday(&start, NULL);
  for (i = 0; i < WORK_SIZE; i++) {
    dest[i] = power(i, i);
  }
  gettimeofday(&end, NULL);
  output_time_difference("simple for loop", &start, &end);

  // PART 2: ------------------------use fork

  gettimeofday(&start, NULL);

  dest = mmap(NULL, sizeof(float) * WORK_SIZE, PROT_READ | PROT_WRITE,
              MAP_SHARED | MAP_ANONYMOUS, -1, 0);

  int fresult = fork();
  if (fresult == 0) {
    // child handles the odd values
    for (i = 1; i < WORK_SIZE; i += 2) {
      dest[i] = power(i, i);
    }
    exit(0);
  } else {
    // parent handles the even values
    for (i = 0; i < WORK_SIZE; i += 2) {
      dest[i] = power(i, i);
    }
    wait(NULL);  // wait for child to finish
  }

  gettimeofday(&end, NULL);
  output_time_difference("fork", &start, &end);

  // PART 3: ------------------------use pthreads

  gettimeofday(&start, NULL);

  pthread_t tid[2];

  int starting1 = 0;
  int starting2 = 1;

  pthread_create(&tid[0], NULL, threadFun, &starting1);

  // lets let the parent do some work as well
  threadFun(&starting2);

  pthread_join(tid[0], NULL);

  gettimeofday(&end, NULL);

  output_time_difference("pthreads", &start, &end);

  return 0;
}
