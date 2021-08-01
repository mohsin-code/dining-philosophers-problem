#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "Monitor.h"

// Dining-Philosophers Solution Using Monitors

//Semaphores
sem_t mutex, next;

//Count of suspended process
int next_count = 0;

enum {THINKING, HUNGRY, EATING} state[5];
typedef struct
{
	sem_t sem;
	//count variable for philosophers waiting on condition semaphore sem
	int count;
} condition;
condition x[N];

// Pickup chopsticks
void pickup(int i)
{
	sem_wait(&mutex);
	state[i] = HUNGRY;
	// set state to eating in test()
	// only if my left and right neighbors
	// are not eating
	test(i);
	if (state[i] != EATING)
		wait(i);
		
	if(next_count > 0)
		sem_post(&next);
	else
		sem_post(&mutex);
}

// Put down chopsticks
void putdown(int i)
{
	sem_wait(&mutex);
	state[i] = THINKING;
	// if right neighbor R=(i+1)%5 is hungry and
	// both of R’s neighbors are not eating,
	// set R’s state to eating and wake it up by
	// signaling R’s CV
	test((i + 1) % 5);
	test((i + 4) % 5);
	
	if(next_count > 0)
		sem_post(&next);
	else
		sem_post(&mutex);
}

void test(int i)
{
	if (	(state[(i + 1) % 5] != EATING) &&
		(state[(i + 4) % 5] != EATING) &&
		(state[i] == HUNGRY)	) {
		// indicate that I’m eating
		state[i] = EATING;
		// signal() has no effect during Pickup(),
		// but is important to wake up waiting
		// hungry philosophers during Putdown()
		signal(i);
	}
}
	
void wait(int i) {
	x[i].count++;
	if (next_count > 0)
		sem_post(&next);  //signal
	else
		sem_post(&mutex); //signal
	sem_wait(&x[i].sem);
	x[i].count--;
}
	
void signal(int i) {
	if (x[i].count > 0) {
		next_count++;
		sem_post(&x[i].sem);
		sem_wait(&next);
		next_count--;
	}
}
	
void initialization() {
	sem_init(&mutex,0,1);
	sem_init(&next,0,0);
	// Execution of Pickup(), Putdown() and test()
	// are all mutually exclusive,
	// i.e. only one at a time can be executing
	for(int i = 0; i < 5; i++){
		state[i] = THINKING;
		sem_init(&x[i].sem,0,0);
		x[i].count = 0;
	// Verify that this monitor-based solution is
	// deadlock free and mutually exclusive in that
	// no 2 neighbors can eat simultaneously
	}
}
