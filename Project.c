// For timers
// https://www.qnx.com/developers/docs/6.4.0/neutrino/getting_started/s1_timer.html


#include <stdlib.h>
#include <stdio.h>
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

#define SHMNAME "/my_shm"
#define SEMNAME "/my_sem"

#define SIZE 10

// The following structure is overlaid on the shared memory.
struct phu {
	 sem_t s;
	 char beginning_of_data[SIZE];
} *p;

char *addr;

int N = 0;
int nbytes;

void* collect1(void){
	puts("Collect 1 Active\n");

    if (sem_wait(&p->s) == -1) {
        printf("Sem_wait error.\n");
    } //end if

    int i = 0;
    while(i < SIZE) {
		addr[i] = 'A';
		printf("C1 i = %d. N = %d.\n", i, N);
		i++;
    } //end while

    if (sem_post(&p->s) == -1) {
        printf("Sem_post error.\n");
    } //end if

	puts("Got into the memory\n");
} //end collect1

void* collect2(void){
	puts("Collect 2 Active\n");

    if (sem_wait(&p->s) == -1) {
        printf("Sem_wait error.\n");
    } //end if

    int i = 0;
    while(i < SIZE) {
		addr[i] = 'A';
		printf("C2 i = %d. N = %d.\n", i, N);
		i++;
    } //end while

    if (sem_post(&p->s) == -1) {
        printf("Sem_post error.\n");
    } //end if

	puts("Got into the memory\n");
} //end collect1

void* reader1(void){
	puts("Reader 1 Active\n");
} //end collect1

void* reader2(void){
	puts("Reader 2 Active\n");
} //end collect1

void* reader3(void){
	puts("Reader 3 Active\n");
} //end collect1


int main(int argc, char *argv[]) {

	int d, nbytes;

    nbytes = sizeof(*p);

    struct timespec {
        long    tv_sec,
                tv_nsec;
    };

    struct itimerspec {
        struct timespec it_value,
                        it_interval;
    };

    it_value.tv_sec = 5;
    it_value.tv_nsec = 500000000;
    it_interval.tv_sec = 0;
    it_interval.tv_nsec = 0;

    int timer_create (clockid_t clock_id, struct sigevent *event, timer_t *timerid);

    // Create shared memory region.
     if ((d = shm_open(SHMNAME, O_RDWR|O_CREAT|O_EXCL, 0666)) == -1) {
          printf("Unable to open shared memory.\n");
          exit(1);
     } //end if

     if (ftruncate(d, nbytes) != 0) {
          close(d);
          shm_unlink(SHMNAME);
          printf("Unable to truncate.\n");
          exit(1);
     } //end if

     p = (struct phu *)mmap(NULL, nbytes, PROT_READ|PROT_WRITE, MAP_SHARED, d, 0);
     if(p == (struct phu *) -1) {
          close(d);
          printf("Unable to mmap.\n");
          exit(1);
     } //end if

     shm_unlink(SHMNAME);

     // Create semaphore.
     if (sem_init(&p->s, 1, 1) == -1) {
          printf("Sema init ERROR = %i.\n", errno);
          exit(1);
     } //end if

     addr = p->beginning_of_data;

    puts("Finished creating data structures\n");

    pthread_create( NULL, NULL, &collect1, (void *) 1 );
    pthread_create( NULL, NULL, &collect2, (void *) 1 );
    pthread_create( NULL, NULL, &reader1, (void *) 1 );
    pthread_create( NULL, NULL, &reader2, (void *) 1 );
    pthread_create( NULL, NULL, &reader3, (void *) 1 );

    pthread_join(2,NULL);
    pthread_join(3,NULL);
    pthread_join(4,NULL);
    pthread_join(5,NULL);
    pthread_join(6,NULL);

    puts("Created threads\n");

    puts("Finished execution\n");

	return 0;

} //end main
