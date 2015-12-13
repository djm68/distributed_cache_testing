/* *****************************************************************************
 *                  Copyright (C) 2010 by RNA Networks, Inc.                  *
 *                         Author: Dominic Maraglia                           *
 *                      dominic.maraglia@rnanetworks.com                      *
 *                                                                            *
 *           Performance timing code provided to Dave for the Sabre POC       *
 *                                                                            *
 **************************************************************************** */

#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "/opt/rnanetworks/inc/pgd.h" 
#define BILLION  1000000000L;

int obj_max, num_threads, value_size;
extern char *optarg;
extern int optind, optopt, opterr;
int	set_barrier=0;
int	iterations=1; 
pthread_barrier_t *barrier;

void set_key(int n, char* buf)
{
        sprintf(buf, "%d", n);
}
 
void* get_keys(void* ptr)
{
        char* value_buf = (char*)malloc(value_size+1);
        uint64_t value_length = 0;
				struct timespec start, stop;
				double elapsed;
        int n, rc, m;
				char buf[value_size+1];
				/* pass 1 -- not timed */
        for(n = 0; n != obj_max; ++n) {
               set_key(n, buf);
               rc = rna_share_get(buf, value_buf, value_size, &value_length);
               if(rc < 0) {
                       fprintf(stderr, "ERROR: %d\n", rc);
               }
        }

				if(set_barrier) {
					printf("Thread witing on barrier\n");
					pthread_barrier_wait(&barrier);
				}

				/* pass 2 -- timing pass */
				clock_gettime(CLOCK_REALTIME, &start);
        for(n = 0; n != obj_max; ++n) {
               set_key(n, buf);
					
        			 for(m = 0; m < iterations; ++m) {
               	rc = rna_share_get(buf, value_buf, value_size, &value_length);
               	if(rc < 0) {
                       	fprintf(stderr, "ERROR: %d\n", rc);
               	}
							 }
        }
				clock_gettime(CLOCK_REALTIME, &stop);
				elapsed = (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / (double)BILLION;
				printf("%d GET operations in:  %lf secs\n", n, elapsed);
				printf("%d GET operations in:  %lf usecs\n", n, (elapsed * 1000000) );
				printf("Average time per GET op:%lf usecs\n", ((elapsed * 1000000) / (n * iterations)) );
        free(value_buf); 
        return 0;
}
 

int main(int argc, char *argv[])
{
	int c;
	char *argval;
  obj_max=10;
  num_threads=1;
	value_size=4096;

	while ((c = getopt(argc, argv, ":o:p:b:i:hx")) != -1) {
			switch(c) {
					case 'h':
							printf("./time_get -o num object -p num threads -i <iterations per object> -x Turn On Barrier -b size of buffer (4096 default)\n");
							exit(0);
							break;
					case 'o':
							argval = optarg;
							obj_max = atoi(argval);
							break;
					case 'p':
							argval = optarg;
							num_threads = atoi(argval);
							break;
					case 'b':
							argval = optarg;
							value_size = atoi(argval);
							break;
					case 'i':
							argval = optarg;
							iterations = atoi(argval);
							break;
					case 'x':
							set_barrier = 1;
							break;
					case ':':
							printf("-%c without arg\n", optopt);
							exit(1);
							break;
					case '?':
							printf("unknown arg %c\n", optopt);
							exit(1);
							break;
					}
		}

	pthread_t threads[num_threads];
	int rc = rna_share_open(NULL);
	if(rc != 0) {
			fprintf(stderr, "COULD NOT CONNECT TO RNA: %d\n", rc);
			return -1;
	}
 
	int n;

	pthread_barrier_init(&barrier, NULL, num_threads);

	for(n = 0; n != num_threads; ++n) {
		pthread_create(&threads[n], NULL, get_keys, NULL);
	}
 
	for(n = 0; n != num_threads; ++n) {
		pthread_join(threads[n], NULL);
	}

	exit (0);
}
