#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/* Global shared data */
int amount = 0; /* Shared data accessed by threads */
pthread_mutex_t lock; /* Mutex for mutual exclusion */

/* Function for deposit threads */
void* deposit(void* param) {
    int val = atoi((char*)param);
    
    pthread_mutex_lock(&lock);
    amount += val;
    printf("Deposit amount = %d\n", amount); /* Output modified value */
    pthread_mutex_unlock(&lock);
    
    pthread_exit(0);
}

/* Function for withdraw threads */
void* withdraw(void* param) {
    int val = atoi((char*)param);
    
    pthread_mutex_lock(&lock);
    amount -= val;
    printf("Withdrawal amount = %d\n", amount); /* Output modified value */
    pthread_mutex_unlock(&lock);
    
    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: ./PLmutex <deposit_amount> <withdraw_amount>\n");
        return -1;
    }

    pthread_t workers[6]; /* 6 threads total */
    pthread_attr_t attr;
    pthread_attr_init(&attr); /* Use default attributes */
    pthread_mutex_init(&lock, NULL);

    /* Create 3 deposit threads */
    for (int i = 0; i < 3; i++) {
        if (pthread_create(&workers[i], &attr, deposit, argv[1]) != 0) {
            perror("Failed to create deposit thread"); /* Error reporting */
        }
    }

    /* Create 3 withdraw threads */
    for (int i = 3; i < 6; i++) {
        if (pthread_create(&workers[i], &attr, withdraw, argv[2]) != 0) {
            perror("Failed to create withdraw thread"); /* Error reporting */
        }
    }

    /* Wait for all threads to finish */
    for (int i = 0; i < 6; i++) {
        pthread_join(workers[i], NULL);
    }

    printf("Final amount = %d\n", amount); /* Correct final amount */
    pthread_mutex_destroy(&lock);
    return 0;
}
