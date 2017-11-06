#include <cstdlib>
#include <iostream>
#include <sys/neutrino.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define run_time 5//42
#define MAX_CUSTOMERS 420
float accum = 0;
int customer_arrival = 0; //Number of clients that arrived/serviced during the day.
int customer_served = 0;
struct timespec start, stop;

/*
typedef struct{
	struct timespec arrival_time;
	struct timespec service_start_time;
	struct timespec departure_time;
}customer;*/

class customer{
public:
	struct timespec arrival_time;
	struct timespec service_start_time;
	struct timespec departure_time;

public:
	customer(){
		clock_gettime(CLOCK_REALTIME, &arrival_time);
		clock_gettime(CLOCK_REALTIME, &service_start_time);
		clock_gettime(CLOCK_REALTIME, &departure_time);
	}
};

customer qeue[MAX_CUSTOMERS+1] = {};
pthread_mutex_t mutexlist;


//General teller function which determines the behavior of the tellers.
static void * teller(void * teller_ID)
{	/*Convert void type pointer to integer type pointer. Necessary in order to
	to access the value contained in the pointer*/
	srand(time(NULL));
	float trans_time = 0;

	int* ID = (int*)teller_ID;
	int data = * ID; // * dereferencing = Getting the value that is stored in the memory
					 // location pointed by the pointer.
	int my_customer = 0;
	printf("Teller %d active!\n", data);


	while(accum <run_time || customer_served < customer_arrival)
	{


		if(customer_served < customer_arrival)  //If customer served is different than customer that arrived (List position)
											 //somebody is on the list, take first customer service time.
		{
		pthread_mutex_lock (&mutexlist);
		customer_served++;
		my_customer = customer_served; //Because I have to stay with the current customer for calculations.
									   //Once list is unlocked, tellers can do anything with index variable.
		//printf("Customer Position Served:%d\n", customer_served);


		pthread_mutex_unlock (&mutexlist);

		clock_gettime(CLOCK_REALTIME, &qeue[my_customer].service_start_time);
		//random time for transaction of each customer.

		trans_time = (float) (((rand()%8)+1)/10.0)*1000000.0;
		usleep (trans_time);
		clock_gettime(CLOCK_REALTIME, &qeue[my_customer].departure_time);
		printf("Teller %d is working with the following customer position: %d\n",data, my_customer);
		}
	}

		printf("Teller: %d done\n", data);

return NULL;
}



int main(int argc, char *argv[]) {
	srand(time(NULL));
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
	clock_gettime(CLOCK_REALTIME, &start);

	double next_customer = 0;
	while(accum <run_time)	//While loop to define the time the bank is going to be open.
	{
		next_customer = (float)(((rand()%4)+1)/10.0)*1000000.0;
		printf("Time for next customer:%f\n",next_customer/1000000.0);
		usleep(next_customer);

		pthread_mutex_lock (&mutexlist);
		customer_arrival++;
		//customer a ={start,start,start};
		//qeue[customer_arrival] = new customer();
		clock_gettime(CLOCK_REALTIME, &qeue[customer_arrival].arrival_time);
		pthread_mutex_unlock (&mutexlist);

		clock_gettime(CLOCK_REALTIME, &stop); // Keep checking the time until condition is fulfilled.

		accum = (stop.tv_sec - start.tv_sec);// Calculation of elapsed time.
		printf("New customer arrival: %d\n",customer_arrival);
	}
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	pthread_join(thread3, NULL);

	printf("Accumulated time:%.0f", accum);
	printf("Total Number of customers serviced during the day: %d\n",customer_served);

	float wait_time=0;
	float avg_wait_time = 0;
	for (int j=1; j<=customer_served;j++)
	{

		wait_time = (float)(((qeue[j].service_start_time.tv_sec +qeue[j].service_start_time.tv_nsec )- (qeue[j].arrival_time.tv_sec+qeue[j].arrival_time.tv_nsec)));
	}

	avg_wait_time = wait_time/customer_served;

	printf("Average waiting time: %.0f\n",avg_wait_time);

	return EXIT_SUCCESS;
}

