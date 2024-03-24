#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define LOWER_NUM 1
#define UPPER_NUM 10000
#define BUFFER_SIZE 100
#define MAX_COUNT 10000

int buffer[BUFFER_SIZE];    // buffer for storing numbers
int buffer_top = 0;         // index to keep track of the top of the buffer
int producer_done = 0;      // flag to indicate the producer has finished
int total_processed = 0;    // total number of all numbers processed
int odd_processed = 0;      // total number of odd numbers processed
int even_processed = 0;     // total number of even numbers processed

// initialize mutex and buffer condition 
pthread_mutex_t mutexBuffer = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffer_not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t buffer_not_empty = PTHREAD_COND_INITIALIZER;

// push item onto the stack buffer
void push(int num) {
    buffer[buffer_top++] = num;
}

// pop item from the stack buffer
int pop() {
    return buffer[--buffer_top];
}

// producer function
void* producer(void *args) {
    FILE *all_file = fopen("all.txt", "w");
    for (int i = 0; i < MAX_COUNT; i++) {
        int num = (rand() % (UPPER_NUM - LOWER_NUM + 1)) + LOWER_NUM;
        
        // wait if the buffer is full
        while (buffer_top >= BUFFER_SIZE);
        
        pthread_mutex_lock(&mutexBuffer);   // lock mutex before accessing buffer
     
        push(num);   // increment buffer index and store the number
        if (all_file != NULL) {
            fprintf(all_file, "%d\n", num);
        }
        total_processed++;
        
        pthread_mutex_unlock(&mutexBuffer);   // unlock mutex after accessing buffer
    }
    
    fclose(all_file);
    producer_done = 1;    // set producer done flag
}

// consumer function
void* consumer(void *args) {
    char *output_file = (char *)args;
    FILE *out_file = fopen(output_file, "a");
    
    while (1) {
        pthread_mutex_lock(&mutexBuffer);    // lock mutex before accessing buffer
       
        // check if producer is done and buffer is empty 
        if (producer_done && buffer_top == 0) {
            pthread_mutex_unlock(&mutexBuffer);    // unlock mutex if condition met 
            break;
        }
        
        int num;
        if (buffer_top > 0) {    // check if the buffer is not empty 
            if ((output_file[0] == 'e' && buffer[buffer_top-1] % 2 == 0) ||
                (output_file[0] == 'o' && buffer[buffer_top-1] % 2 != 0)) {
                // decrement buffer index and return the top number
                num = pop();    
                fprintf(out_file, "%d\n", num);
                
                // increment processed count based on parity 
                if (output_file[0] == 'e') {
                    even_processed++;
                } else {
                    odd_processed++;
                }
            }
        }
      
        pthread_mutex_unlock(&mutexBuffer);    // unlock mutex after accessing buffer
    }
    
    fclose(out_file);
    return NULL;
}

int main() {
    pthread_t producer_thread, even_consumer_thread, odd_consumer_thread;
    
    clock_t start_time = clock();   // start time
    
    // create producer and consumer threads
    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&even_consumer_thread, NULL, consumer, "even.txt");
    pthread_create(&odd_consumer_thread, NULL, consumer, "odd.txt");
    
    // wait for all threads to finish
    pthread_join(producer_thread, NULL);
    pthread_join(even_consumer_thread, NULL);
    pthread_join(odd_consumer_thread, NULL);
    
    clock_t end_time = clock();    // end time
    
    double time_elapsed = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;

    printf("Program has finished successfully! \n");
    printf("Total lines processed: %d\n", total_processed);
    printf("Total odd numbers processed: %d\n", odd_processed);
    printf("Total even numbers processed: %d\n", even_processed);
    printf("Time elapsed: %f seconds\n", time_elapsed);
    
    return 0;
}
