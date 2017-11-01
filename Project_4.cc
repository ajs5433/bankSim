#include <cstdlib>
#include <iostream>
#include <sys/neutrino.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define run_time 42
#define MAX_CUSTOMERS 420
double accum = 0;
struct customer
{
	timespec arrival_time;
	timespec service_start_time;
	timespec departure_time;
};

customer qeue [MAX_CUSTOMERS] = {0};
pthread_mutex_t mutexlist;


//General teller function which determines the behavior of the tellers.
static void * teller(void * teller_ID)
{	/*Convert void type pointer to integer type pointer. Necessary in order to
	to access the value contained in the pointer*/
	int* ID = (int*)teller_ID;
	int data = * ID; // * dereferencing = Getting the value that is stored in the memory
					 // location pointed by the pointer.
	printf("Teller %d active!\n", data);
	//pthread_mutex_lock (&mutexlist);
	//pthread_mutex_unlock (&mutexlist);


//	for (int j=0; j<5; j++)
//	{
//		printf("Hello World!\n");
//		usleep(1000000);
// 			}

return NULL;
}
//
//int elapsed_time(){
//	struct timespec now;
//	int elapsed_time = start.tv_sec - now.tv_sec;
//	return elapsed_time;
//}


int main(int argc, char *argv[]) {

	std::cout << "Welcome to the QNX Momentics IDE" << std::endl;
	pthread_mutex_init (&mutexlist, NULL);
	pthread_t thread1, thread2, thread3;

	// Attributes to be fed to the pthread_create function used to ID each teller.
	int attr_1 = 1;
	int attr_2 = 2;
	int attr_3 = 3;

	//Creation of 3 different tellers.
	pthread_create(&thread1, NULL, teller, &attr_1);
	pthread_create(&thread2, NULL, teller, &attr_2);
	pthread_create(&thread3, NULL, teller, &attr_3);

	/*Join each teller to the main thread to ensure the program doesn't finish
	until all threads are done*/
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	pthread_join(thread3, NULL);


	struct timespec start, stop;
	clock_gettime(CLOCK_REALTIME, &start);


	double next_customer = 0;
	while(accum <run_time)	//While loop to define the time the bank is going to be open.
	{
		next_customer = (((rand()%4)+1)/10.0)*100000;
		usleep(next_customer);

		clock_gettime(CLOCK_REALTIME, &stop); // Keep checking the time until condition is fulfilled.
		accum = (stop.tv_sec - start.tv_sec);// Calculation of elapsed time.
		printf("Next customer in line:%.1f\n", next_customer);
		usleep(1000000);
	}

	printf("Accumulated time:%.0f", accum);


	return EXIT_SUCCESS;
}

