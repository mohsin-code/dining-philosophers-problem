#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "Monitor.h"

void* philospher(void* num)
{
 
    while (1) {
 
        int* i = num;
 
	sleep(1);
 	printf("\nPhilosopher %d is thinking for 1 second - %d", *i, getpid());
	pickup(*i);
  	printf("\nPhilosopher %d is eating for 1 second - %d", *i, getpid());
      	sleep(1);
    	putdown(*i);
    }
}
 
int main()
{
 
    int i, phil[N];
    pthread_t thread_id[N];
 
    // initialize the semaphores
    initialization();
 
    for (i = 0; i < N; i++) 
    {
	phil[i] = i;
	// create philosopher processes
	pthread_create(&thread_id[i], NULL, philospher, &phil[i]);
 	printf("Philosopher %d is thinking\n", i + 1);
}
 
    for (i = 0; i < N; i++)
        pthread_join(thread_id[i], NULL);
        
        return 0;
}
