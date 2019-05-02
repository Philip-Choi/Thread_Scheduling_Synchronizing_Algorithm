#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include "bbuff.h"
#include "stats.h"

//candy struct
typedef struct candy {
	int factory_number;
	double time_stamp_in_ms;
} candy_t;

double current_time_in_ms(void) {
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	return now.tv_sec * 1000.0 + now.tv_nsec/1000000.0;
}

candy_t* create_candy (int i) {

    //dynamically allocate candy 
    candy_t* candy = malloc (sizeof (candy_t));
    candy -> factory_number = i;
    candy -> time_stamp_in_ms = current_time_in_ms();

    return candy;
}

_Bool stop_thread = false;

void* candy_produce_function (void* i) {
	int factory_num = (intptr_t)i;

	while(!stop_thread) {
		int seconds_to_wait = rand()%3;
		printf("\tFactory %d ships candy & waits %ds\n", factory_num, seconds_to_wait);

		candy_t* candy = create_candy (factory_num);

		bbuff_blocking_insert(candy);

		stats_record_produced(factory_num);

		sleep(seconds_to_wait);
	}

	printf("Candy-factory %d done\n", factory_num);
	
	return NULL;
}

void* candy_consume_function() {	

	for(;;) {
		candy_t* candy = bbuff_blocking_extract();
	
		if(candy!=NULL) {
			stats_record_consumed(candy->factory_number, current_time_in_ms()-candy->time_stamp_in_ms);
		}
	
		free (candy);
		int seconds_to_wait = rand()%1;
		sleep(seconds_to_wait);
	}
	// printf("Candy was consumed");
	return NULL;
}

int main(int argc, char* argv[]) {	

    // 1.  Extract arguments
    int factories = 0;
    int kids = 0;
    int seconds = 0;

    if (argc != 4) {

        printf("Error: invalid arguments\n");
        exit(1);
    }

    factories = atoi(argv[1]);
    kids = atoi(argv[2]);
    seconds = atoi(argv[3]);

    if (factories <= 0 || kids <= 0  || seconds <= 0 ) {

        printf("Error: invalid arguments input\n");
        exit(1);
    }

    // printf("factories: %d \n", factories);
    // printf("kids: %d \n", kids);
    // printf("seconds: %d \n", seconds);
	
	//printf ("step 1 Works \n");
	
	//2. Initialize modules
	bbuff_init();
	stats_init(factories);
	srand(time(NULL));

	//printf ("step 2 Works \n");

	//3. Launch candy-factory threads
	pthread_t* factory_thread_array = malloc(factories *(sizeof(pthread_t)));
	pthread_t factory_thread_id;

	for (int i = 0; i < factories; i++) {

	    pthread_attr_t factory_attr;
        pthread_attr_init (&factory_attr); // default factory attributes
        pthread_create (&factory_thread_id, &factory_attr, candy_produce_function, (void*)(intptr_t)i);
        factory_thread_array[i] = factory_thread_id;
	}

	//printf ("step 3 Works \n");

	//4. Launch kid threads
	pthread_t* kid_thread_array = malloc(kids *(sizeof(pthread_t)));
	pthread_t kid_thread_id;

	for (int i = 0; i < factories; i++) {

	    pthread_attr_t kid_attr;
        pthread_attr_init (&kid_attr); // default factory attributes
        pthread_create (&kid_thread_id, &kid_attr, candy_consume_function, NULL);
        kid_thread_array[i] = kid_thread_id;
	}

	//printf ("step 4 Works \n");

	//5. Wait for requested time
	for(int i = 0; i < seconds; i++) {
		sleep(1);
		printf("Time: %ds\n", i);
	}
	
	//printf ("step 5 Works \n");

	//6. Stop candy-factory threads
	stop_thread = true;
	
	for(int i = 0; i < factories; i++) {
		pthread_join(factory_thread_array[i], NULL);
	}

	//printf ("step 6 Works \n");

	//7. Wait until no more candy
	while(bbuff_is_empty() == false) {
		printf ("Waiting for all candy to be consumed \n");
		sleep(1);
	}
	
	//printf ("step 7 Works \n");

	//8. Stop kid threads
	for(int i = 0; i < kids; i++) {

		pthread_cancel (kid_thread_array[i]);
		pthread_join (kid_thread_array[i], NULL);
	}

	//printf ("step 8 Works \n");

	//9. Print statistics
	printf("Statistics: \n");
	stats_display();

	//printf ("step 9 Works \n");

	//10. Cleanup any allocated memory
	stats_cleanup();

	free(factory_thread_array);
	free(kid_thread_array);
	factory_thread_array = NULL;
	kid_thread_array = NULL;
	//printf ("step 10 Works \n");
	return 0;
}