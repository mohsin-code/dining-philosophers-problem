/**
 * @file philosopher_monitor.c
 * @brief Implementation of a Hoare-style Monitor for resource arbitration.
 * * This module solves the Dining Philosophers problem using a monitor pattern
 * implemented with POSIX semaphores. It ensures deadlock-freedom and
 * mutual exclusion for shared resource access.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_PHILOSOPHERS 5
#define RUN_TIME_SEC 10

typedef enum { THINKING, HUNGRY, EATING } state_t;

typedef struct {
    sem_t sem;
    int count; // Number of threads
} condition_t;

typedef struct {
    state_t state[NUM_PHILOSOPHERS];
    condition_t cond_vars[NUM_PHILOSOPHERS];
    sem_t mutex;
    sem_t next;
    int next_count;
} PhilosopherMonitor;

static void monitor_signal(PhilosopherMonitor *mon, int i) {
    if (mon->cond_vars[i].count > 0) {
        mon->next_count++;
        sem_post(&mon->cond_vars[i].sem);
        sem_wait(&mon->next);
        mon->next_count--;
    }
}

static void monitor_wait(PhilosopherMonitor *mon, int i) {
    mon->cond_vars[i].count++;
    if (mon->next_count > 0)
        sem_post(&mon->next);
    else
        sem_post(&mon->mutex);
    
    sem_wait(&mon->cond_vars[i].sem);
    mon->cond_vars[i].count--;
}

static void test_neighbors(PhilosopherMonitor *mon, int i) {
    int left = (i + NUM_PHILOSOPHERS - 1) % NUM_PHILOSOPHERS;
    int right = (i + 1) % NUM_PHILOSOPHERS;

    if ((mon->state[left] != EATING) && 
        (mon->state[right] != EATING) && 
        (mon->state[i] == HUNGRY)) {
        
        mon->state[i] = EATING;
        monitor_signal(mon, i);
    }
}

void monitor_init(PhilosopherMonitor *mon) {
    sem_init(&mon->mutex, 0, 1);
    sem_init(&mon->next, 0, 0);
    mon->next_count = 0;

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        mon->state[i] = THINKING;
        sem_init(&mon->cond_vars[i].sem, 0, 0);
        mon->cond_vars[i].count = 0;
    }
}

void pickup_chopsticks(PhilosopherMonitor *mon, int i) {
    sem_wait(&mon->mutex);
    
    mon->state[i] = HUNGRY;
    test_neighbors(mon, i);
    
    if (mon->state[i] != EATING) {
        monitor_wait(mon, i);
    }

    if (mon->next_count > 0)
        sem_post(&mon->next);
    else
        sem_post(&mon->mutex);
}

void putdown_chopsticks(PhilosopherMonitor *mon, int i) {
    sem_wait(&mon->mutex);
    
    mon->state[i] = THINKING;
    
    // Check if neighbors can now eat
    test_neighbors(mon, (i + NUM_PHILOSOPHERS - 1) % NUM_PHILOSOPHERS);
    test_neighbors(mon, (i + 1) % NUM_PHILOSOPHERS);

    if (mon->next_count > 0)
        sem_post(&mon->next);
    else
        sem_post(&mon->mutex);
}

void* philosopher_routine(void* arg) {
    int id = *(int*)arg;
    PhilosopherMonitor* mon = (PhilosopherMonitor*)((void**)arg)[1];

    for (int loops = 0; loops < 3; loops++) { // Limited loops for demo
        printf("Philosopher %d is thinking...\n", id);
        sleep(1);

        printf("Philosopher %d is HUNGRY.\n", id);
        pickup_chopsticks(mon, id);

        printf("Philosopher %d is EATING.\n", id);
        sleep(1);

        printf("Philosopher %d is finished eating.\n", id);
        putdown_chopsticks(mon, id);
    }
    return NULL;
}

int main() {
    pthread_t threads[NUM_PHILOSOPHERS];
    int ids[NUM_PHILOSOPHERS];
    void* args[NUM_PHILOSOPHERS][2];
    PhilosopherMonitor mon;

    monitor_init(&mon);

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        ids[i] = i;
        args[i][0] = &ids[i];
        args[i][1] = &mon;
        pthread_create(&threads[i], NULL, philosopher_routine, args[i]);
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Simulation complete.\n");
    return 0;
}
