/*
 ============================================================================
 Name        : A2.c
 Author      : Fady Abousifein 
 Student No. : 400506836
 Course      : SFWRENG 3SH3 – Operating Systems
 Assignment  : Assignment 2 – Sleeping Teaching Assistant
 ============================================================================
 Compile:
     gcc -pthread A2.c -o A2

 Run:
     ./A2 <number_of_students>

 NOTE: Generative AI was used to help with formatting code and using the best 
       practices for multithreading applications. No functional code used to 
       solve the assignment was generated with any AI. 
 ============================================================================
*/

// Libraries and defined variables
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define NUM_CHAIRS 3
#define PROGRAMMING_TIME 3
#define HELP_TIME 2

// Global variables
int waiting_students = 0;        
int total_students = 0;

pthread_mutex_t chair_mutex;     
sem_t students_sem;              
sem_t ta_sem;                    

// Function forward declarations
void random_sleep(int max_seconds);
void* ta_thread(void* arg); 
void* student_thread(void* arg); 
void initialize(); 
void cleanup();

int main(int argc, char* argv[]) {
    
    // Handle improper running command
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_students>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    // Parse n, and store it
    total_students = atoi(argv[1]);
   
    // Handle edge case (0 students / negative students)
    if (total_students <= 0) {
        fprintf(stderr, "Number of students must be positive.\n");
        exit(EXIT_FAILURE);
    }
    
    // Generate random seed and initalize mutexes/semaphores and threads
    srand(time(NULL));
    pthread_t ta;
    pthread_t* students;
    int* student_ids;
    initialize();
    
    // Allocate memory for studnets 
    students = malloc(sizeof(pthread_t) * total_students);
    student_ids = malloc(sizeof(int) * total_students);
    
    // Error handling 
    if (!students || !student_ids) {
        fprintf(stderr, "Memory allocation failed.\n");
        cleanup();
        exit(EXIT_FAILURE);
    }
    
    // Create TA thread
    if (pthread_create(&ta, NULL, ta_thread, NULL) != 0) {
        fprintf(stderr, "Failed to create TA thread.\n");
        cleanup();
        exit(EXIT_FAILURE);
    }
    
    // Create n student threads (Based on user input)
    for (int i = 0; i < total_students; i++) {
        student_ids[i] = i + 1;

        if (pthread_create(&students[i], NULL,
                           student_thread, &student_ids[i]) != 0) {
            fprintf(stderr, "Failed to create student thread %d.\n", i + 1);
            cleanup();
            exit(EXIT_FAILURE);
        }
    }
    
    // Join threads
    for (int i = 0; i < total_students; i++) {
        pthread_join(students[i], NULL);
    }
    pthread_join(ta, NULL);
    
    // Cleanup any threads and memory
    free(students);
    free(student_ids);
    cleanup();

    return 0;
}

// Function generates a random sleep time between 1 and max seconds
void random_sleep(int max_seconds) {
    int duration = (rand() % max_seconds) + 1;
    sleep(duration);
}

// Function handles the logic for the sleeping TA
void* ta_thread(void* arg) {
    (void)arg; // avoid compiler being annoying 

    while (1) {
        // TA remains asleep until a student is waiting
        sem_wait(&students_sem);

        // Lock the critical section, and update wating_student count 
        pthread_mutex_lock(&chair_mutex);

        waiting_students--;
        printf("TA: Ready to help a student. Students waiting = %d\n", waiting_students);
         
        // Tell waiting student TA is ready 
        sem_post(&ta_sem);

        pthread_mutex_unlock(&chair_mutex);
        
        // Random time the TA takes to help the student 
        random_sleep(HELP_TIME);
    }

    return NULL;
}

// Function handles the logic for the waiting students
void* student_thread(void* arg) {
    int id = *((int*)arg);

    while (1) {
        // Student programming (not yet needs help)
        printf("Student %d: Programming...\n", id);
        random_sleep(PROGRAMMING_TIME);
        
        // Student needs help and aquires the lock (Outside TA office)
        printf("Student %d: Needs help.\n", id);
        pthread_mutex_lock(&chair_mutex);

        if (waiting_students < NUM_CHAIRS) {
            // Sit in the chair and wait 
            waiting_students++;
            printf("Student %d: Sitting in hallway. Waiting = %d\n",
                   id, waiting_students);

            // Tell the TA someone is waiting
            sem_post(&students_sem);
            pthread_mutex_unlock(&chair_mutex);
            
            // Wait until the TA is ready before going in 
            sem_wait(&ta_sem);

            printf("Student %d: Receiving help from TA.\n", id);
        }
        else {
            // No Chairs available (Time to use chat GPT :)
            printf("Student %d: No chairs available. Will try later.\n", id);
            pthread_mutex_unlock(&chair_mutex);
        }
    }

    return NULL;
}

// Function handles the initialization of mutex and semaphores  
void initialize() {
    pthread_mutex_init(&chair_mutex, NULL);
    sem_init(&students_sem, 0, 0);
    sem_init(&ta_sem, 0, 0);
}

// Function handles the destruction of mutex and semaphores
void cleanup() {
    pthread_mutex_destroy(&chair_mutex);
    sem_destroy(&students_sem);
    sem_destroy(&ta_sem);
}
