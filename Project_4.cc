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
float accum = 0;
float teller_break = 0;
int customer_arrival = 0; //Number of clients that arrived/serviced during the day.
int customer_served = 0;
struct timespec start, stop;
float teller_idle_total[3] = {0.0,0.0,0.0};
float max_teller_idle_time[3] = {0.0,0.0,0.0};
float avg_break_time[3]={0.0,0.0,0.0};
float number_of_breaks[3] = {0.0,0.0,0.0};
float max_break_duration[3] = {0.0,0.0,0.0};
float min_break_duration[3]={0.4,0.4,0.4};
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

customer qeue[MAX_CUSTOMERS+1];
pthread_mutex_t mutexlist;


//General teller function which determines the behavior of the tellers.
static void * teller(void * teller_ID)
{	/*Convert void type pointer to integer type pointer. Necessary in order to
	to access the value contained in the pointer*/
	srand(time(NULL));
	float trans_time = 0;
	struct timespec last_break, time_now, teller_idle_start, teller_idle_stop ;

	int* ID = (int*)teller_ID;
	int data = * ID; // * dereferencing = Getting the value that is stored in the memory
					 // location pointed by the pointer.
	int my_customer = 0;
	printf("Teller %d active!\n", data);


	clock_gettime(CLOCK_REALTIME, &last_break); //Initializing last break for first loop.

	float next_break = (((rand()%30)+30)/10.0);  //Next break from 30 min to 60 min.

	while(accum <run_time)  //While bank is open.
	{

		//while NOT customer being attended.
		clock_gettime(CLOCK_REALTIME, &teller_idle_start); //take time when idle state started.
		while (!(customer_served < customer_arrival) && accum <run_time)
		{
			//Do nothing
		}
		clock_gettime(CLOCK_REALTIME, &teller_idle_stop); //take time when idle state ended.

		//Calculate current idle time:
		float teller_idle_now = (float)((teller_idle_stop.tv_sec + teller_idle_stop.tv_nsec/1000000000) - (teller_idle_start.tv_sec + teller_idle_start.tv_nsec/1000000000));
		//Calculate total idle time to later calculate the average.
		teller_idle_total[data-1] = teller_idle_total[data-1] + teller_idle_now;

			//Logic for maximum teller idle time.
			if(teller_idle_now > max_teller_idle_time[data-1])
			{
				max_teller_idle_time[data-1] = teller_idle_now;
			}

		//while there are customers in line and being attended
		while(customer_served < customer_arrival && accum <run_time)
		{
			clock_gettime(CLOCK_REALTIME, &time_now);		//take current time to calculate when next break has to happen.
			if((float)(time_now.tv_sec - last_break.tv_sec) >= next_break)
			{
				float break_duration = (((rand()%4)+1)/10.0)*1000000.0;		//Break duration in microseconds.
				//printf("---->>Teller %d is taking a break\n",data);
				number_of_breaks [data-1]++;		//Used to accumulate the total number of breaks of each teller on the same array.
				usleep(break_duration);				//sleep for established break duration.
				clock_gettime(CLOCK_REALTIME, &last_break); //time in which last break ended.

				//take the break time duration for each teller and save it to calculate the average time later.
				avg_break_time[data-1] = avg_break_time[data-1]+(break_duration/1000000);

				next_break = (((rand()%30)+30)/10.0);  //Next break from 30 min to 60 min.

				if(break_duration/1000000 > max_break_duration[data-1])    //Max break duration.
				{
					max_break_duration[data-1] = break_duration/1000000;
				}
				if(break_duration/1000000 < min_break_duration[data-1])		//Min break duration.
				{
					min_break_duration[data-1] = break_duration/1000000;
				}
			}



			if(customer_served < customer_arrival)  //If customer served is different than customer that arrived (List position)
												 //somebody is on the list, take first customer service time.
			{
			pthread_mutex_lock (&mutexlist); //lock list.
			customer_served++;				// increment number of customers served by one. (customer served index)
			my_customer = customer_served; //Because I have to stay with the current customer for calculations.
										   //Once list is unlocked, tellers can do anything with index variable.
			//printf("Customer Position Served:%d\n", customer_served);


			pthread_mutex_unlock (&mutexlist); //unlock list.

			clock_gettime(CLOCK_REALTIME, &(qeue[my_customer].service_start_time)); //take the time when the transaction starts.

			//random time for transaction of each customer.
			trans_time = (float) (((rand()%8)+1)/10.0)*1000000.0;
			usleep (trans_time);	//sleep for the establish transaction time.
			clock_gettime(CLOCK_REALTIME, &(qeue[my_customer].departure_time));  //take the time when the transaction is done.
			//printf("Teller %d is working with the following customer position: %d\n",data, my_customer);

			}
			//printf("----->>transaction time: %f\n", trans_time);
		}
	}

		printf("Teller: %d done\n", data);  //indicate when each teller is done.

return NULL;
}



int main(int argc, char *argv[]) {
	srand(time(NULL));
	//std::cout << "Welcome to the QNX Momentics IDE" << std::endl;
	pthread_mutex_init (&mutexlist, NULL);	//initialize function to lock/unlock list.
	pthread_t thread1, thread2, thread3; //thread declaration.

	// Attributes to be fed to the pthread_create function used to ID each teller.
	int attr_1 = 1;
	int attr_2 = 2;
	int attr_3 = 3;

	//Creation of 3 different tellers.
	pthread_create(&thread1, NULL, teller, &attr_1);
	pthread_create(&thread2, NULL, teller, &attr_2);
	pthread_create(&thread3, NULL, teller, &attr_3);


	clock_gettime(CLOCK_REALTIME, &start); //Bank opening.

	double next_customer = 0;
	int max_queue = 0;
	while(accum <run_time)	//While loop to define the time the bank is going to be open.
	{
		next_customer = (float)(((rand()%4)+1)/10.0)*1000000.0; //Random time for next customer.
		//printf("Time for next customer:%f\n",next_customer/1000000.0);
		usleep(next_customer); //wait time for next customer in microseconds.

		pthread_mutex_lock (&mutexlist);//Lock list so it can only be accessed by one teller at the time.
		customer_arrival++;			//increment customer arrival.

		//Logic for maximum number of customers in queue.
		if (customer_arrival-customer_served > max_queue)
		{
			max_queue = customer_arrival-customer_served;
		}


		clock_gettime(CLOCK_REALTIME, &(qeue[customer_arrival].arrival_time)); //Mark the arrival time of each customer and place them in their corresponding position.
		pthread_mutex_unlock (&mutexlist);  //unlock list.

		clock_gettime(CLOCK_REALTIME, &stop); // Keep checking the time to calculate the elapsed time.

		accum = (stop.tv_sec - start.tv_sec);// Calculation of elapsed time in order to determine closing time..
		//printf("New customer arrival: %d\n",customer_arrival);
	}

	/*Join each teller to the main thread to ensure the program doesn't finish
		until all threads are done*/
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	pthread_join(thread3, NULL);

	///Calculation for the average teller idle time.
	float total_teller_idle_time = teller_idle_total[0] + teller_idle_total[1] + teller_idle_total[2];
	float avg_teller_idle_time = total_teller_idle_time / 3;

	//Number of breaks taken by each teller.
	float breaks_teller_1 = number_of_breaks[0];
	float breaks_teller_2 = number_of_breaks[1];
	float breaks_teller_3 = number_of_breaks[2];

	//Calculation for the average break time for each teller.
	float avg_break_time_teller_1 = avg_break_time[0]/breaks_teller_1;
	float avg_break_time_teller_2 = avg_break_time[1]/breaks_teller_2;
	float avg_break_time_teller_3 = avg_break_time[2]/breaks_teller_3;

	//Logic for maximum single teller idle time.
	float max_single_teller_idle_time = 0;
	for (int i=0; i<3; i++)
	{
		if(max_teller_idle_time[i]>max_single_teller_idle_time)
		{
			max_single_teller_idle_time = max_teller_idle_time[i];
		}

	}

	printf("Accumulated time:%.0f\n", accum);
	printf("1- Total Number of customers serviced during the day: %d\n",customer_served);

	// In this section the calculations for average time in queue and transaction time
	float avg_transaction_time = 0;
	float avg_wait_time = 0;
	float wait_time_in_line = 0;
	float wait_time_transaction = 0;
	float max_wait_time= 0;
	float current_cus_wait_time;
	float max_transaction_time = 0;
	float current_transaction_time = 0;

	for (int j=1; j<=customer_served; j++)
	{
		//Calculate current customer wait time in time.
		current_cus_wait_time = (float)(((qeue[j].service_start_time.tv_sec+qeue[j].service_start_time.tv_nsec/1000000000) - (qeue[j].arrival_time.tv_sec+qeue[j].arrival_time.tv_nsec/1000000000)));
		//Total wait time in line to later calculate average.
		wait_time_in_line = wait_time_in_line + current_cus_wait_time;

		//Logic for max waiting time.
		if (current_cus_wait_time > max_wait_time)
		{
			max_wait_time = current_cus_wait_time;
		}

		//+qeue[j].departure_time.tv_nsec/1000000000
		//+qeue[j].service_start_time.tv_nsec/1000000000

		//Calculate curren transaction time.
		current_transaction_time = (float)(((qeue[j].departure_time.tv_sec - (qeue[j].service_start_time.tv_sec))));
		//Total transaction time to later calcualte average.
		wait_time_transaction = wait_time_transaction + current_transaction_time;

		//Logic for maximum transaction time.
		if (current_transaction_time >= max_transaction_time)
		{
			max_transaction_time = current_transaction_time;
		}
	}

	avg_wait_time = wait_time_in_line/(float)(customer_served);   //Average wait time.
	avg_transaction_time = wait_time_transaction/(float)(customer_served);  //Average transaction time.


	printf("2- Average waiting time in queue: %f\n",avg_wait_time);
	printf("3- Average time customer spends with teller (Transaction time): %f\n",avg_transaction_time);
	printf("4- Average time tellers wait for customers (Teller Idle Time): %f\n",avg_teller_idle_time);
	printf("5- Maximum customer wait time in queue: %f\n",max_wait_time);
	printf("6- Maximum wait time for tellers waiting for customers: %f\n",max_single_teller_idle_time);
	printf("7- Maximum transaction time: %f\n",max_transaction_time);
	printf("8- Maximum depth of the customer queue: %d\n", max_queue);
	printf("9.a - Number of breaks for each of the three tellers: %f - %f - %f\n", breaks_teller_1,breaks_teller_2,breaks_teller_3);
	printf("9.b - Average break time for each of the three tellers: %f - %f - %f\n", avg_break_time_teller_1,avg_break_time_teller_2,avg_break_time_teller_3);
	printf("9.c - Longest break time for each of the three tellers: %f - %f - %f\n", max_break_duration[0],max_break_duration[1],max_break_duration[2]);
	printf("9.d - Shortest break time for each of the three tellers: %f - %f - %f\n", min_break_duration[0],min_break_duration[1],min_break_duration[2]);
	return EXIT_SUCCESS;
}

