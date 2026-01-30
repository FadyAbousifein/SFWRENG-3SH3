#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

int amount = 0;
pthread_mutex_t lock;
sem_t empty; /* For condition amount >= 400 [cite: 106] */
sem_t full;  /* For condition amount <= 0 [cite: 106] */

void* deposit(void* param) {
    printf("Executing deposit function\n"); /* Start message [cite: 110] */
    
    sem_wait(&empty); /* Block if amount >= 400 [cite: 99] */
    pthread_mutex_lock(&lock);
    
    amount += 100; /* Fixed amount [cite: 100] */
    printf("Amount after deposit = %d\n", amount); /* Modified value [cite: 109] */
    
    pthread_mutex_unlock(&lock);
    sem_post(&full); /* Signal that money is available to withdraw */
    
    pthread_exit(0);
}

void* withdraw(void* param) {
    printf("Executing Withdraw function\n"); /* Start message [cite: 111] */
    
    sem_wait(&full); /* Block if amount <= 0 [cite: 98] */
    pthread_mutex_lock(&lock);
    
    amount -= 100; /* Fixed amount [cite: 100] */
    printf("Amount after Withdrawal = %d\n", amount); /* Modified value [cite: 109] */
    
    pthread_mutex_unlock(&lock);
    sem_post(&empty); /* Signal space for more deposits */
    
    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./PLsem 100\n");
        return -1;
    }

    pthread_t workers[10]; /* 10 threads total [cite: 101] */
    pthread_mutex_init(&lock, NULL);

    /* Initial values: 
       empty = 4 (Max 400 balance / 100 per deposit)
       full = 0 (Starts at 0 balance) [cite: 107] */
    sem_init(&empty, 0, 4); 
    sem_init(&full, 0, 0);

    /* Create 7 deposit and 3 withdraw threads [cite: 101, 102] */
    for (int i = 0; i < 7; i++) {
        pthread_create(&workers[i], NULL, deposit, argv[1]);
    }
    for (int i = 7; i < 10; i++) {
        pthread_create(&workers[i], NULL, withdraw, argv[1]);
    }

    for (int i = 0; i < 10; i++) {
        pthread_join(workers[i], NULL); /* Parent waits for all [cite: 114] */
    }

    printf("Final amount = %d\n", amount); /* Should be 400 [cite: 113] */

    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&lock);
    return 0;
}
