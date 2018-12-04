// For timers
// https://www.qnx.com/developers/docs/6.4.0/neutrino/getting_started/s1_timer.html

/*================================================================= /
 * Known Problems
 * 1: Not all libraries are needed
 * 2: Readers must run faster than the collect tasks
 *
  ================================================================*/

/*================================================================= /
 * ECE 4251 Lab Project
 * Author: Ben Wedemire
 * Student ID: 3515624
 * Date: 6/12/2018
 * Description: Global variable initializations.
  =================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>			//needed for rand
#include <errno.h>			//needed for semaphores
//#include <string.h>
#include <unistd.h>
//#include <limits.h>
#include <semaphore.h>
#include <fcntl.h> 			//needed for shared memory
#include <sys/mman.h> 		//needed for shared memory
#include <sys/types.h>
#include <signal.h>
//#include <math.h>
//#include <float.h>
#include <pthread.h>		//needed for threads
#include <time.h>			//needed for timer
#include <sys/siginfo.h>	//needed for timer
#include <stdint.h>			//needed for unsigned int8
#include <sys/netmgr.h>		//needed for signals/IRQ
#include <sys/neutrino.h>	//needed for signals/IRQ

#define SIZE 10
#define MY_PULSE_CODE _PULSE_CODE_MINAVAIL
#define Other _PULSE_CODE_MAXAVAIL

uint8_t shmem1[SIZE];	//Creating shared memory
uint8_t shmem2[SIZE];	//Creating shared memory
uint8_t shmem3[SIZE];	//Creating shared memory
uint8_t collect1[SIZE];	//Creating shared memory
uint8_t collect2[SIZE];	//Creating shared memory

uint8_t *shmem1_temp;	//Current location in shmem1
uint8_t *shmem2_temp;	//Current location in shmem2
uint8_t *shmem3_temp;	//Current location in shmem3
uint8_t *collect1_temp;	//Current location in collect1
uint8_t *collect2_temp;	//Current location in collect2

sem_t m1;				//semaphore for shared memory 1
sem_t m2;				//semaphore for shared memory 2
sem_t m3;				//semaphore for shared memory 3
sem_t r1;				//semaphore for data available for reader 1
sem_t r2;				//semaphore for data available for reader 2
sem_t r3;				//semaphore for data available for reader 3

typedef union {
        struct _pulse   pulse;
} my_message_t;

int chid;               //chan
int rcvid;        		// receive id
my_message_t msg;		// message object

int chid2;               //chan
int rcvid2;        		// receive id
my_message_t msg2;		// message object


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

	uint8_t temp;
	int i = 0;

    while(1){
		rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL); //wait for message on the channel
		if (rcvid == 0) { /* we got a pulse */
			if (msg.pulse.code == MY_PULSE_CODE) {
				printf("we got a pulse from our timer\n");
			} //end if
		} //end if

		temp = *collect1_temp;
		if(collect1_temp == collect1 + 9){
			collect1_temp = collect1;
		} //end if
		else collect1_temp++;

		if (sem_wait(&m1) == -1) {
			printf("Sem_wait error.\n");
		} //end if

		if (i < SIZE) {
			*shmem1_temp = temp;
			printf("C1 i = %d. shmem1 = %d.\n", i, *shmem1);
			//i++;
		} //end if
		else puts("Reached end of memory\n");

		if (sem_post(&m1) == -1) {
			printf("Sem_post error.\n");
		} //end if

		if (sem_post(&r1) == -1) {
			printf("Sem_post error.\n");
		} //end if

		puts("Got into the memory\n");
	} //end while
    return 0;
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

	uint8_t temp;
	int i = 0;

	while(1){
		rcvid2 = MsgReceive(chid2, &msg2, sizeof(msg2), NULL); //wait for message on the channel
		if (rcvid2 == 0) { /* we got a pulse */
			if (msg2.pulse.code == MY_PULSE_CODE) {
				printf("we got a pulse from our second timer\n");
			} //end if
		} //end if

		temp = *collect2_temp;
		if(collect2_temp == collect2 + 9){
			collect2_temp = collect2;
		} //end if
		else collect2_temp++;

		if (sem_wait(&m2) == -1) {
			printf("Sem_wait error.\n");
		} //end if

		if (i < SIZE) {
			shmem2[i] = temp;
			printf("C2 i = %d. shmem2 = %d.\n", i, *shmem2);
			//i++;
		} //end if
		else puts("Reached end of memory\n");

		if (sem_post(&m2) == -1) {
			printf("Sem_post error.\n");
		} //end if

		if (sem_post(&r2) == -1) {
			printf("Sem_post error.\n");
		} //end if

		puts("Got into the memory\n");
	} //end while
	return 0;
} //end collect1

/*================================================================= /
 * ECE 4251 Lab Project
 * Author: Ben Wedemire
 * Student ID: 3515624
 * Date: 28/11/2018
 * Description: First Reader thread. Waits to receive data from
 * 				Collector 1, displays the received data to the
 * 				console then sends the data to Reader 3.
  =================================================================*/

void* reader1_task(void){
	puts("Reader 1 Active\n");
	uint8_t data_temp = 0;
	while(1){
		sem_wait(&r1);				//Wait for the something to be in shmem1
		sem_wait(&m1);				//Wait for shared memory to be free

		data_temp = *shmem1_temp;

		sem_post(&m1);				//Wait for shared memory to be free
		printf("Data received was %d\n", data_temp);

		sem_wait(&m3);				//Wait for shared memory to be free

		*shmem3_temp = data_temp;

		sem_post(&m3);				//Free shared memory
		sem_post(&r3);				//Signal there is something in shared memory

	} //end while
	return 0;
} //end reader1_task

/*================================================================= /
 * ECE 4251 Lab Project
 * Author: Ben Wedemire
 * Student ID: 3515624
 * Date: 28/11/2018
 * Description: Second Reader thread. Waits to receive data from
 * 				Collector 2, displays the received data to the
 * 				console then sends the data to Reader 3.
  =================================================================*/

void* reader2_task(void){
	puts("Reader 2 Active\n");
	uint8_t data_temp = 0;
	while(1){
		sem_wait(&r2);				//Wait for the something to be in shmem1
		sem_wait(&m2);				//Wait for shared memory to be free

		data_temp = *shmem2_temp;

		sem_post(&m2);				//Wait for shared memory to be free
		printf("Reader 2 received %d\n", data_temp); //this is for debug
		data_temp = data_temp >> 4;	//shifting right by 1 byte

		sem_wait(&m3);				//Wait for shared memory to be free

		*shmem3_temp = data_temp;

		sem_post(&m3);				//Free shared memory
		sem_post(&r3);				//Signal there is something in shared memory


	} //end while
	return 0;
} //end reader2_task

/*================================================================= /
 * ECE 4251 Lab Project
 * Author: Ben Wedemire
 * Student ID: 3515624
 * Date: 28/11/2018
 * Description: Third Reader thread. Receives data from both Reader
 * 				1 and Reader 2, then outputs on the LEDs
  =================================================================*/

void* reader3_task(void){
	puts("Reader 3 Active\n");
	uint8_t data_temp = 0;
	while(1){
		sem_wait(&r3);				//Wait something to be in shared memory
		sem_wait(&m3);				//Wait for shared memory to be free

		data_temp = *shmem3_temp;
		if(shmem3_temp == shmem3 + 9){
			shmem3_temp = shmem3;
		} //end if
		else shmem3_temp++;

		sem_post(&m3);				//Free shared memory
		printf("Reader 3 got: %d\n", data_temp);

	} //end while
	return 0;
} //end reader3_task

/*================================================================= /
 * ECE 4251 Lab Project
 * Author: Ben Wedemire
 * Student ID: 3515624
 * Date: 28/11/2018
 * Description: Main program. Creates two collector threads and
 * 				three Reader threads. A total of three shared
 * 				memory blocks are used.
  =================================================================*/

int main(int argc, char *argv[]) {

	shmem1_temp = shmem1;
	shmem2_temp = shmem2;
	shmem3_temp = shmem3;
	collect1_temp = collect1;
	collect2_temp = collect2;

     // Create semaphore 1
     if (sem_init(&m1, 0, 1) == -1) {
          printf("Sema init ERROR = %i.\n", errno);
          exit(1);
     } //end if

     // Create semaphore 2
     if (sem_init(&m2, 0, 1) == -1) {
          printf("Sema init ERROR = %i.\n", errno);
          exit(1);
     } //end if

     // Create semaphore 3
     if (sem_init(&m3, 0, 1) == -1) {
          printf("Sema init ERROR = %i.\n", errno);
          exit(1);
     } //end if

     // Create semaphore 4
     if (sem_init(&r1, 0, 0) == -1) {
          printf("Sema init ERROR = %i.\n", errno);
          exit(1);
     } //end if

     // Create semaphore 5
     if (sem_init(&r2, 0, 0) == -1) {
          printf("Sema init ERROR = %i.\n", errno);
          exit(1);
     } //end if

     // Create semaphore 6
     if (sem_init(&r3, 0, 0) == -1) {
          printf("Sema init ERROR = %i.\n", errno);
          exit(1);
     } //end if

    //Filling Collect1 & Collect2 with rand data

    int i = 0;
    while(i < SIZE){
    	collect1[i] = rand();
    	printf("Value put into collect1 is: %d\n", collect1[i]);
    	collect2[i] = rand();
    	printf("Value put into collect2 is: %d\n", collect2[i]);
    	i++;
    } //end

    puts("Finished creating data structures\n");

    pthread_create( NULL, NULL, &reader1_task, (void *) 1 );
    pthread_create( NULL, NULL, &reader2_task, (void *) 1 );
    pthread_create( NULL, NULL, &reader3_task, (void *) 1 );

	struct sigevent event;              //create signal event struct
	struct itimerspec itime;     		//create timer struct
	timer_t timer_id;        			//timer object


	int result;
	// Create Channel
	chid = ChannelCreate(0);
	SIGEV_INTR_INIT(&event);        //Add interrupt ability to the signal event
	InterruptAttachEvent(10,&event,0); // attach the IRQ10 with the signal event interrupt with no flag
	//event.sigev_intr_init()
	event.sigev_notify = SIGEV_PULSE;        //the notification of the signal event is pulse type
	event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0,chid,_NTO_SIDE_CHANNEL, 0); //the channel on which the signal will be listening is the created channel
	event.sigev_priority = getprio(0);        //priority of 0
	event.sigev_code = MY_PULSE_CODE;        //the code that executes upon the signal event is MY_PULSE_CODE
	timer_create(CLOCK_MONOTONIC, &event, &timer_id);        //create a monotonic timer which signals the event of timerid

	itime.it_value.tv_sec = 1;
	/* 500 million nsecs = .5 secs */
	itime.it_value.tv_nsec = 500000000;
	itime.it_interval.tv_sec = 1;
	/* 500 million nsecs = .5 secs */
	itime.it_interval.tv_nsec = 500000000;
	timer_settime(timer_id, 0, &itime, NULL);

	pthread_t tid;

	pthread_create( &tid, NULL, &collect1_task, (void *) 1 );
	//----------------------------------------------------------------------------------------------------
	struct sigevent         event2;         //create signal event struct
	struct itimerspec       itime2;     	//create timer struct
	timer_t                 timer_id2;      //timer object


	int result2;
	// Create Channel
	chid2 = ChannelCreate(0);
	SIGEV_INTR_INIT(&event2);        //Add interrupt ability to the signal event
	InterruptAttachEvent(9,&event2,0); // attach the IRQ10 with the signal event interrupt with no flag
	//event.sigev_intr_init()
	event2.sigev_notify = SIGEV_PULSE;        //the notification of the signal event is pulse type
	event2.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0,chid2,_NTO_SIDE_CHANNEL, 0); //the channel on which the signal will be listening is the created channel
	event2.sigev_priority = getprio(0);        //priority of 0
	event2.sigev_code = MY_PULSE_CODE;        //the code that executes upon the signal event is MY_PULSE_CODE
	timer_create(CLOCK_MONOTONIC, &event2, &timer_id2);        //create a monotonic timer which signals the event of timerid

	itime2.it_value.tv_sec = 1;
	/* 500 million nsecs = .5 secs */
	itime2.it_value.tv_nsec = 500000000;
	itime2.it_interval.tv_sec = 4;
	/* 500 million nsecs = .5 secs */
	itime2.it_interval.tv_nsec = 500000000;
	timer_settime(timer_id2, 0, &itime2, NULL);
	pthread_t tid2;

	pthread_create( &tid2, NULL, &collect2_task, (void *) 1 );

	puts("Finished creating timers\n");

    pthread_join(2,NULL);
    pthread_join(3,NULL);
    pthread_join(4,NULL);
    pthread_join(5,NULL);
    pthread_join(6,NULL);

    puts("Created threads\n");

    puts("Finished main\n");

	return 0;

} //end main
