#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <stdbool.h>

#include "bbuff.h"

void* candy_bounded_buffer[BUFFER_SIZE];

sem_t mutex;
sem_t full;
sem_t empty;

int index;

void bbuff_init(void)
{
	sem_init(&mutex, 0, 1);	// mutex initialized to 1; pshared argument is zero to share between threads of the proess
	sem_init(&full, 0, 0); // full initialized to 0
	sem_init(&empty, 0, BUFFER_SIZE); // empty initialized to N, which is BUFFER_SIZE

	index = BUFFER_SIZE;
}

void bbuff_blocking_insert(void* item)
{
	sem_wait(&empty);
	sem_wait(&mutex);

	candy_bounded_buffer[BUFFER_SIZE - index] = item;
	index--;

	sem_post(&mutex);
	sem_post(&full);

	return;
}

void* bbuff_blocking_extract(void)
{
	sem_wait(&full);
	sem_wait(&mutex);
	
	int extract_index = BUFFER_SIZE - (index+1);

	void* candy;
	candy = candy_bounded_buffer[extract_index];
	candy_bounded_buffer[extract_index] = NULL;
	index++;

	sem_post(&mutex);
	sem_post(&empty);

	return candy;
}

_Bool bbuff_is_empty(void)
{
	if (index == 10) {
		return true;
	}

	return false;
}

