#include <stdlib.h>
#include <stdio.h>
#include <float.h>

#include "stats.h"

int num_candy_producers;

typedef struct candy_statistics {

	int candy_created;
	int candy_consumed;

	double min_delay;
	double max_delay;
	double total_delay;
} statistics_t;

statistics_t* stat;

void stats_init(int num_producers) {

	num_candy_producers = num_producers;
	stat = malloc (num_producers * sizeof(statistics_t));

	for(int i = 0; i< num_candy_producers; i++) {

		stat[i].candy_created = 0;
		stat[i].candy_consumed = 0;
		stat[i].min_delay = DBL_MAX;
		stat[i].max_delay = DBL_MIN;
		stat[i].total_delay = 0;
	}

}

void stats_cleanup(void) {

	free(stat);
	stat = NULL;
}

void stats_record_produced(int factory_number) {

	stat[factory_number].candy_created ++;
}

void stats_record_consumed(int factory_number, double delay_in_ms) {

	stat[factory_number].candy_consumed = stat[factory_number].candy_consumed + 1;

		if(stat[factory_number].min_delay > delay_in_ms) {
			stat[factory_number].min_delay = delay_in_ms;
		}
		if(stat[factory_number].max_delay < delay_in_ms) {
			stat[factory_number].max_delay = delay_in_ms;
		}

		stat[factory_number].total_delay = stat[factory_number].total_delay + delay_in_ms;
}

void stats_display(void) {

	double average_delay[num_candy_producers];

	printf("%8s%10s%10s%15s%15s%15s\n", "Factory#", "#Made", "#Eaten", "Min Deplay[ms]", "Avg Delay[ms]", "Max Delay[ms]");

	for(int i=0; i<num_candy_producers; i++) {

		if (stat[i].candy_consumed > 0) {
			average_delay[i] = ((stat[i].total_delay) / (stat[i].candy_consumed)) ;
		} else if (stat[i].candy_consumed == 0) {
			average_delay[i] = 0;
		}

		if (stat[i].min_delay == DBL_MAX) {
			stat[i].min_delay = 0;
		}

		if (stat[i].max_delay == DBL_MIN) {
			stat[i].max_delay = 0;
		}

		if (stat[i].candy_created != stat[i].candy_consumed) {
			printf ("ERROR: Mismatch between number made and eaten.\n");
			break;
		}

		else {
			printf("%8d%10d%10d%15.5f%15.5f%15.5f\n", i, stat[i].candy_created, stat[i].candy_consumed, stat[i].min_delay, average_delay[i], stat[i].max_delay);
		
		}

	}
}
