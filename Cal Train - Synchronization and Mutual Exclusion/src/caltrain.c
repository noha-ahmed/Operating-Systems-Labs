#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "caltrain.h"



void
station_init(struct station *station)
{
	station->freeSeats = 0;
	station->waitingPassengers = 0;
	station->boardingPassengers = 0;
	pthread_mutex_init(&station->train_mutex, NULL);
	pthread_cond_init(&station->cond_arrive, NULL); 
	pthread_cond_init(&station->cond_leave, NULL); 
}

void
station_load_train(struct station *station, int count)
{
	
	 //fprintf(stderr,"%d\n",station->waitingPassengers);
	 //fprintf(stderr,"%d\n",station->freeSeats);
	// aquire the lock
	pthread_mutex_lock(&station->train_mutex);

	/*if( station->waitingPassengers == 0 || count == 0 ){
		pthread_mutex_unlock( &station->train_mutex );
		return;
	} */

	// update free seats in station
	station->freeSeats += count;

	// wake up the sleeping threads of waitining passengers if exist
	// does not release the lock yet

	//check if either train is full or no waiting passengers
	while( station->freeSeats != 0 && station->waitingPassengers !=0 ){
		// wait until no more free seats or no more waiting passengers
		// sleep and release the lock then aquire the lock again when signalled on the  same condition variable
		pthread_cond_broadcast(&station->cond_arrive);
		pthread_cond_wait( &station->cond_leave , &station->train_mutex );
	}

	station->freeSeats = 0;

	//release the lock and leave the station
	pthread_mutex_unlock( &station->train_mutex );

}

void
station_wait_for_train(struct station *station)
{
	// aquire the lock
	pthread_mutex_lock(&station->train_mutex);

	// update  waiting passengers in station
	station->waitingPassengers++;

	// check if there exist seat available
	while( station->freeSeats == 0 || station->boardingPassengers == station->freeSeats ){
		// wait until there is a free seat
		// sleep and release the lock then aquire the lock again when signalled on the  same condition variable 
		// wakes up when a train arrived 
		pthread_cond_wait( &station->cond_arrive , &station->train_mutex );
	}

	station->boardingPassengers++;
	station->waitingPassengers--;

	//release the lock to other passengers
	pthread_mutex_unlock( &station->train_mutex );
	

}

void
station_on_board(struct station *station)
{
	// aquire the lock
	pthread_mutex_lock(&station->train_mutex);

	
	station->boardingPassengers--;
	station->freeSeats--;
	if( station->freeSeats == 0 || ( station->boardingPassengers == 0 && station->waitingPassengers == 0) ){
		pthread_cond_signal( &station->cond_leave );
	}

	//release the lock to the train or the passengers
	pthread_mutex_unlock( &station->train_mutex );
}
