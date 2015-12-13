/* *****************************************************************************
 *                  Copyright (C) 2010 by RNA Networks, Inc.                  *
 *                         Author: Dominic Maraglia                           *
 *                      dominic.maraglia@rnanetworks.com                      *
 *                                                                            *
 **************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h> 
#include <unistd.h> 
#include <pthread.h>
#include <inttypes.h>
#include <sys/types.h>
#include "/opt/rnanetworks/inc/pgd.h"
#define STRD_SIZE 16

/* Function defs */
void usage();
void parse_args(int argc, char *argv[]);
void *cache_operations(void *num);
int gen_rand_str(char *obj);
int validate(char *big_obj, void *buffer, char *key);
int rna_destroy(char *key);
int rna_put(char *big_obj, char *key, int ttl, int len);
int rna_get(char *key, void *buffer, uint64_t buffer_size, uint64_t object_size);


/* Globals to save pain of passing many cmd line args */
int sleep_flag, nouniq_flag, norand_flag, put_flag, get_flag, del_flag, obj_max, bytes_max, loop_max, ttl, num_threads, put_get_ratio;
extern char *optarg;
extern int optind, optopt, opterr;
pthread_mutex_t ssl_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[])
{
	srand(time(0));			/* Initialize rand seed */

	ttl=0;								/* TTL for obj; 0 = forever */
	obj_max=10;						/* num of keys to put/get/del */
	loop_max=1;						/* num of times to loop put/get/del */
	num_threads=1;				/* default thread count */
	put_get_ratio=1;			/* number of get ops per put op */
	bytes_max=STRD_SIZE;	/* max size of object to create */

	parse_args(argc, argv);		/* parse options passed on the cmd line */
	int rc = rna_share_open(NULL);	/* Initialize Object Caching interface */
	if (rc < 0)
		{
			printf("Error: failed to init RNA OCI\n");
			exit(rc);
		}

	pthread_t threads[num_threads];		/* Init for num_threads */

	long n;					/* Create threads */
	for(n=0; n < num_threads; n++)
	{
		rc = pthread_create( &threads[n], NULL, cache_operations, (void *)n);
		if (rc) 
			{
				printf("Error return code from pthread_create() is %d\n", rc);
				exit(-1);
			}
		else 
			printf("Thread Create success: %d  ThreadNUM %d\n", rc, n);
	}

	for(n=0; n < num_threads; n++)		/* Join threads */
		{
			rc = pthread_join( threads[n], NULL);
			if (rc) 
				{
					printf("Error return code from pthread_join() is %d\n", rc);
					exit(-1);
				}
			else
				printf("Thread Join success: %d  ThreadNUM %d\n", rc, n);
		}  
	exit(0);
}

/* Parse command line args */
void parse_args(int argc, char *argv[])
{
	int c;
  char *argval;
	if (argc < 2)
		 usage();
	while ((c = getopt(argc, argv, ":Rhpgdso:b:l:t:r:")) != -1) {
			switch(c) {
					case 'h':
							usage();
							break;
					case 'R':
							norand_flag++;
							break;
					case 'g':
							get_flag++;
							break;
					case 'd':
							del_flag++;
							break;
					case 'p':
							put_flag++;
							break;
					case 's':
							sleep_flag++;
							break;
					case 'o':
							argval = optarg;
							obj_max = atoi(argval);
							break;
					case 'b':
							argval = optarg;
							bytes_max = atoi(argval);
							if ( bytes_max < STRD_SIZE )
								bytes_max = STRD_SIZE;
							break;
					case 'r':
							argval = optarg;
							put_get_ratio = atoi(argval);
							break;
					case 'l':
							argval = optarg;
							loop_max = atoi(argval);
							break;
					case 'e':
							argval = optarg;
							ttl = atoi(argval);
							break;
					case 't':
							argval = optarg;
							num_threads = atoi(argval);
							break;
					case ':':
							printf("-%c without arg\n", optopt);
							usage();
							break;
					case '?':
							printf("unknown arg %c\n", optopt);
							usage();
							break;
					}
		}
		return;
}


void usage()
{
	printf("verbs_exr -h -R -p -g -d -s -o num -l num -b num -r num -e num -t num\n");
	printf("   -h Help; this menu\n");
	printf("   -R disable random object size; objects will be exactly -b num bytes\n");
	printf("   -p flag put objects\n");
	printf("   -g flag get objects\n");
	printf("   -d flag destroy objects\n");
	printf("   -s flag sleep 100ms before put/get ops\n");
	printf("   -o num objects to create per loop (default 10)\n");
	printf("   -l num of loops i.e.: put/get object n number of times\n");
	printf("   -b max bytes per object (default 1024)\n");
	printf("   -r ratio of get ops per put op (default 1)\n");
	printf("   -e expire time for objects (ttl) (default 0)\n");
	printf("   -t number of threads (default 1)\n");
	printf("\n   Default behaviour:\n");
	printf("     One thread, one loop, 10 objects\n");
	printf("     One get operation per put object \n");
	printf("     Object sizes are random upto -b bytes\n");
	printf("     Object names are NOT unique.  Objects are named:  key:loop_number:obj_number\n");
	exit(0);
}

/* Logic to determine cache ops to perform */
void *cache_operations(void *num)
{
	struct timespec interval, remainder; 
	interval.tv_sec = 0;
	interval.tv_nsec = 100000000;  /* 100000000 = 100ms */

	long tid;
	int loop_count=0;
	int put_err=0;
	int get_err=0; 
	int del_err=0;
	int cmp_err=0;
	char key[128];
	char host[128];
	uint64_t bytes_put=0;

	/* get hostname for obj key name */
	host[127] = '\0';
	gethostname(host, 127);

	/* get thread id for obj key name */
	tid = pthread_self();

	/* Allocate memory for buffer */
	void *buffer;
	uint64_t object_size;
	uint64_t buffer_size = bytes_max+1;
	buffer = malloc(buffer_size);
	if (buffer == NULL)
		{
			printf("Buffer memory not allocated.\n");
			exit(1);
		}

		/* Allocate memory for object */
		char *big_obj = (char *)malloc(buffer_size);
		if (big_obj == NULL)
			{
				printf("Object memory not allocated.\n");
				exit(1);
			}

			while ( loop_count < loop_max )						/* main loop */
				{
					printf("tid %ld Start Main Loop %d of %d\n", tid, loop_count, loop_max);

					if (put_flag)
						{
							int len, obj_count=0;									/* put loop */
							while ( obj_count < obj_max )
								{
									/*
									 if (nouniq_flag)									* Create obj key w/o loop num - keys will be reused in loop 2+ *
										sprintf(key, "key:%s:%ld:%d", host, tid, obj_count);
									else															* Create obj key w/ loop num - keys will be uniq in each loop *
										sprintf(key, "key:%s:%ld:%d:%d", host, tid, loop_count, obj_count);
									*/

									sprintf(key, "key:%d:%d", loop_count, obj_count); /* Create obj key of loop count and obj count*/
								
								/*	printf("tid %ld ", tid); */
									if ( obj_count == 0 )							/* only gen a new object for obj_count 0 - save rand str time */
										len = gen_rand_str(big_obj);
									big_obj[0]= (char) obj_count;			/* replace first element with (char) of obj_count - uniqs each obj */ 

									if ( sleep_flag )
										nanosleep(&interval, &remainder);

									if (rna_put(big_obj, key, ttl, len) == 0)		/* put object into cache */
										bytes_put+=(len - 1);
									else 
										put_err++;

									++obj_count;
									big_obj[0]='\0';                           	/* Reset obj buffer - set first val to null */
								}
						}

					if (get_flag)
						{
							int get_count=0;
							while ( get_count < put_get_ratio )			/* get each object  put_get_ratio times */
								{
									int obj_count=0;
									while ( obj_count < obj_max )
										{
											sprintf(key, "key:%d:%d", loop_count, obj_count); /* Create obj key of loop count and obj count*/

										/* 	printf("tid %ld ", tid); */

											if ( sleep_flag )
												nanosleep(&interval, &remainder);

											if ( rna_get(key, buffer, buffer_size, object_size) == 0)
												{
													++get_count;
												}
												else
													++get_err;

											++obj_count; /*increment obj count for next loop */
										}
								} /* end put_get_ratio */
						} /* end get */

					/* Object Destroy routine */
					if (del_flag) 
						{
							printf("Start destroy keys\n");
							int rc, obj_count=0;
							while ( obj_count < obj_max )
								{
									sprintf(key, "key:%d:%d", loop_count, obj_count); /* Create obj key of loop count and obj count*/
									rc = rna_share_destroy(key);
									("tid %ld ", tid);
									if (rc != 0)
										{
											printf("Error DEL key: %s with rc %d\n", key, rc);
											del_err++;
										}
									obj_count++;
								}
						}

					++loop_count;
				} /* end loop_max */


	printf("tid %ld TOTAL BYTES PUT: %llu\n", tid, bytes_put);
	printf("tid %ld TOTAL ERROR PUT: %d  GET: %d  DEL: %d  COMP: %d\n",tid, put_err, get_err, del_err, cmp_err);

	free(big_obj);
	free(buffer);
	pthread_exit(NULL);
}


/* put object with *key */
int rna_put(char *big_obj, char *key, int ttl, int len)
{
	int fail_flag=1;	/* Assume failure */
	int rc;
	rc = rna_share_put(key, big_obj, len, ttl);
	if (rc == 0)
		/* printf("Success PUT key: %s of size %d with ttl of %d with rc %d\n", key, len, ttl, rc); */
		fail_flag=0;
	else
		printf("Error PUT key: %s of size %d with ttl of %d with rc %d\n", key, len, ttl, rc);
	return rc;
}


/* Get object for *key */
int rna_get(char *key, void *buffer, uint64_t buffer_size, uint64_t object_size)
{
	int fail_flag=1;	/* Assume failure */
	int rc = rna_share_get(key, buffer, buffer_size, &object_size);
	if (rc == object_size) 
		{
			/* printf("Success GET key: %s of size %d with rc %d\n", key, object_size, rc); */
			fail_flag=0;
		}
	else
		printf("Error GET key: %s of size %d with rc %d\n", key, object_size, rc);
	return fail_flag;
}


/* destroy object for *key */
int rna_destroy(char *key)
{
	int fail_flag=1;	/* Assume failure */
	int rc = rna_share_destroy(key);

	if (rc == 0)
		/* printf("Success DEL key: %s with rc %d\n", key, rc); */
		fail_flag=0;
	else
		printf("Error DEL key: %s with rc %d\n", key, rc);
	return rc;
}


/* Gen a rand string of size in bytes */
int gen_rand_str(char *obj)
{
	static const char text[] = "abcdefghijklmnopqrstuvwxyz0123456789"
														 "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	int nstride, i;
	int stride_size = STRD_SIZE;
	char *buf_ptr;
	char stride[stride_size+1];
	memset(obj, '\0', sizeof(obj));

	for ( i = 0; i < stride_size; ++i )                 /* gen rnd str */
		stride[i] = text[rand() % (sizeof text - 1)];
	stride[i] = '\0';

	if (norand_flag)                                    /* Rand size object or fixed? */ 
		nstride = (int) ( bytes_max / stride_size );      /* fixed object size */
	else
		nstride = ( rand() % ( bytes_max / stride_size) );/* Rand object size */

	if ( (strlen(obj)) >= bytes_max )
		{
			printf("WARN: Object exceeded bytes_max -- will truncate to 1 stride.\n");
			nstride = 1;
		}

	if ( nstride < 1 )                                  /* Make sure we write at least 1 stride */
		nstride = 1;
	for(i = 0; i < nstride; i++)                        /* Write i strides to buffer */
		strncat(obj, stride, stride_size);

	obj[(strlen(obj)+1)] = '\0';
	return(strlen(obj)+1);
}
