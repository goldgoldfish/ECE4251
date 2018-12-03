// For timers
// https://www.qnx.com/developers/docs/6.4.0/neutrino/getting_started/s1_timer.html

/*================================================================= /
 * ECE 4251 Lab Project
 * Author: Ben Wedemire
 * Student ID: 3515624
 * Date: 6/12/2018
 * Description: Global variable initializations.
  =================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>		//needed for rand
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <semaphore.h>
#include <fcntl.h> 		//needed for shared memory
#include <sys/mman.h> 	//needed for shared memory
#include <sys/types.h>
#include <signal.h>
#include <math.h>
#include <float.h>
#include <pthread.h>	//needed for threads
#include <time.h>		//needed for timer
#include <sys/siginfo.h>//needed for timer
#include <stdint.h>		//needed for unsigned int8

#define SIZE 10

uint8_t shmem1[SIZE];
uint8_t shmem2[SIZE];
uint8_t shmem3[SIZE];
uint8_t collect1[SIZE];
uint8_t collect2[SIZE];

uint8_t *shmem1_temp;
uint8_t *shmem2_temp;
uint8_t *shmem3_temp;
uint8_t *collect1_temp;
uint8_t *collect2_temp;

sem_t m1;
sem_t m2;
sem_t m3;
sem_t R3_rdy;

/*================================================================= /
 * ECE 4251 Lab Project
 * Author: Ben Wedemire
 * Student ID: 3515624
 * Date: 6/12/2018
 * Description: First Collector thread. Collects from global array 
                and sends the data to Reader 1. 
                It collects in XX time intervals.
  =================================================================*/

void* collect1_task(void){
	puts("Collect 1 Active\n");
    if (sem_wait(&m1) == -1) {
        printf("Sem_wait error.\n");
    } //end if

    uint8_t temp = *collect1_temp;

    int i = 0;
    while(i < SIZE) {
		shmem1[i] = temp;
		printf("C1 i = %d. shmem1 = %d.\n", i, *shmem1);
		i++;
    } //end while

    if (sem_post(&m1) == -1) {
        printf("Sem_post error.\n");
    } //end if

	puts("Got into the memory\n");
} //end collect1

/*================================================================= /
 * ECE 4251 Lab Project
 * Author: Ben Wedemire
 * Student ID: 3515624
 * Date: 6/12/2018
 * Description: Second Collector thread. Collects from global array 
                and sends the data to Reader 2. 
                It collects in XX time intervals.
  =================================================================*/

void* collect2_task(void){
	puts("Collect 2 Active\n");

	uint8_t temp = *collect2_temp;

    if (sem_wait(&m2) == -1) {
        printf("Sem_wait error.\n");
    } //end if

    int i = 0;
    while(i < SIZE) {
    	shmem2[i] = temp;
		printf("C2 i = %d. shmem2 = %d.\n", i, *shmem2);
		i++;
    } //end while

    if (sem_post(&m2) == -1) {
        printf("Sem_post error.\n");
    } //end if

	puts("Got into the memory\n");
} //end collect1

/*================================================================= /
 * ECE 4251 Lab Project
 * Author: Chris Thomas
 * Student ID: 3506872
 * Date: 28/11/2018
 * Description: First Reader thread. Waits to receive data from Collector 1, displays
 * 				the received data to the console then sends the data to Reader 3.
  =================================================================*/

void* reader1_task(void){
	puts("Reader 1 Active\n");
} //end collect1

void* reader2_task(void){
	puts("Reader 2 Active\n");
} //end collect1

void* reader3_task(void){
	puts("Reader 3 Active\n");
} //end collect1


int main(int argc, char *argv[]) {

	shmem1_temp = shmem1;
	shmem2_temp = shmem2;
	shmem3_temp = shmem3;
	collect1_temp = collect1;
	collect2_temp = collect2;

    struct timespec {
        long    tv_sec,
                tv_nsec;
    };

    struct itimerspec {
        struct timespec it_value,
                        it_interval;
    };

//    it_value.tv_sec = 5;
//    it_value.tv_nsec = 500000000;
//    it_interval.tv_sec = 0;
//    it_interval.tv_nsec = 0;

    int timer_create (clockid_t clock_id, struct sigevent *event, timer_t *timerid);

     // Create semaphore 1
     if (sem_init(&m1, 1, 1) == -1) {
          printf("Sema init ERROR = %i.\n", errno);
          exit(1);
     } //end if

     // Create semaphore 2
     if (sem_init(&m2, 1, 1) == -1) {
          printf("Sema init ERROR = %i.\n", errno);
          exit(1);
     } //end if

     // Create semaphore 3
     if (sem_init(&m3, 1, 1) == -1) {
          printf("Sema init ERROR = %i.\n", errno);
          exit(1);
     } //end if

     // Create semaphore 4
     if (sem_init(&R3_rdy, 1, 1) == -1) {
          printf("Sema init ERROR = %i.\n", errno);
          exit(1);
     } //end if

    //Filling Collect1 & Collect2 with rand data

    time_t t;
    srand((unsigned) time(&t));

    int i = 0;
    while(i < SIZE){
    	collect1[i] = rand();
    	printf("Value put into collect1 is: %d\n", collect1[i]);
    	collect2[i] = rand();
    	printf("Value put into collect2 is: %d\n", collect2[i]);
    	i++;
    } //end

    puts("Finished creating data structures\n");

    pthread_create( NULL, NULL, &collect1_task, (void *) 1 );
    pthread_create( NULL, NULL, &collect2_task, (void *) 1 );
    pthread_create( NULL, NULL, &reader1_task, (void *) 1 );
    pthread_create( NULL, NULL, &reader2_task, (void *) 1 );
    pthread_create( NULL, NULL, &reader3_task, (void *) 1 );

    pthread_join(2,NULL);
    pthread_join(3,NULL);
    pthread_join(4,NULL);
    pthread_join(5,NULL);
    pthread_join(6,NULL);

    puts("Created threads\n");

    puts("Finished execution\n");

	return 0;

} //end main
