/* *****************************************************************************
 *                  Copyright (C) 2010 by RNA Networks, Inc.                  *
 *                         Author: Dominic Maraglia                           *
 *                      dominic.maraglia@rnanetworks.com                      *
 *                                                                            *
 **************************************************************************** */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h> 
#include <unistd.h> 
#include <pthread.h>
#include <inttypes.h>
#include <sys/types.h>
#include "/opt/rnanetworks/inc/pgd.h"
#define STRD_SIZE 64
#define PUT 1
#define GET 2
#define DEL 3

/* Function defs */
void usage();
void gen_key_name(char *key, char *host, long tid, int obj_count);
void parse_args(int argc, char *argv[]);
void *cache_operations(void *opr);
int gen_rand_str(char *obj);
int validate(char *big_obj, void *buffer, char *key);
int rna_destroy(char *key, long tid);
int rna_put(char *big_obj, char *key, int ttl, int leni, long tid);
int rna_get(char *key, void *buffer, uint64_t buffer_size, uint64_t object_size, long tid);


/* Globals to save pain of passing many cmd line args */
int get_thrds, put_thrds, del_thrds, clean_flag, sleep_flag, norand_flag, obj_max, bytes_max, loop_max, ttl, put_get_ratio, host_uniq, cluster_uniq;
extern char *optarg;
extern int optind, optopt, opterr;
pthread_mutex_t ssl_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[])
{
	srand(time(0));			/* Initialize rand seed */

	ttl=0;								/* TTL for obj; 0 = forever */
	obj_max=10;						/* num of keys to put/get/del */
	loop_max=1;						/* num of times to loop put/get/del */
	get_thrds=0;
	put_thrds=0;
	del_thrds=0;	
	put_get_ratio=1;			/* number of get ops per put op */
	bytes_max=STRD_SIZE;	/* max size of object to create */
	int max_thrds;				/* max count for thread pool */

	parse_args(argc, argv);		/* parse options passed on the cmd line */
	int rc = rna_share_open(NULL);	/* Initialize Object Caching interface */
	if (rc < 0)
		{
			printf("Error: failed to init RNA OCI\n");
			exit(rc);
		}

	max_thrds = ( get_thrds + put_thrds + del_thrds );
	pthread_t threads[max_thrds];		/* Init for max_thrds */
	long n=0;					/* Thread Counter */

	while ( put_thrds >= 1 )
	{
		rc = pthread_create( &threads[n], NULL, cache_operations, (void *) PUT);
		if (rc) 
			{
				printf("Error return code from pthread_create() is %d\n", rc);
				exit(-1);
			}
		else 
				printf("PUT Thread Create success: %d  ThreadNUM %d\n", rc, n);
		put_thrds--;
		n++;
	}

	while ( get_thrds >= 1 )
	{
		rc = pthread_create( &threads[n], NULL, cache_operations, (void *) GET);
		if (rc) 
			{
				printf("Error return code from pthread_create() is %d\n", rc);
				exit(-1);
			}
		else 
				printf("GET Thread Create success: %d  ThreadNUM %d\n", rc, n);
		get_thrds--;
		n++;
	}

	while ( del_thrds >= 1 )
	{
		rc = pthread_create( &threads[n], NULL, cache_operations, (void *) DEL);
		if (rc) 
			{
				printf("Error return code from pthread_create() is %d\n", rc);
				exit(-1);
			}
		else 
				printf("DEL Thread Create success: %d  ThreadNUM %d\n", rc, n);
		del_thrds--;
		n++;
	}
		

	for(n=0; n < max_thrds; n++)		/* Join threads */
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
	while ((c = getopt(argc, argv, ":RUuhcp:g:d:so:b:l:r:")) != -1) {
			switch(c) {
					case 'h':
							usage();
							break;
					case 'R':
							norand_flag++;
							break;
					case 'U':
							cluster_uniq++;
							break;
					case 'u':
							host_uniq++;
							break;
					case 'g':
							argval = optarg;
							get_thrds = atoi(argval);
							break;
					case 'd':
							argval = optarg;
							del_thrds = atoi(argval);
							break;
					case 'p':
							argval = optarg;
							put_thrds = atoi(argval);
							break;
					case 's':
							sleep_flag++;
							break;
					case 'c':
							 clean_flag++;
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
	printf("verbs_exr -h -R -p num -g num -d num -o num -l num -b num -r num -e num -t num -s\n");
	printf("   -h Help; this menu\n");
	printf("   -R disable random object size; objects will be exactly -b num bytes\n");
	printf("   -p n num of put threads\n");
	printf("   -g n num of get threads\n");
	printf("   -d n num of destroy threads\n");
	printf("   -s flag sleep 100ms before put/get ops\n");
	printf("   -c clean flag, destroy all keys post loop\n");
	printf("   -o num objects to create per loop (default 10)\n");
	printf("   -l num of loops i.e.: put/get object n number of times\n");
	printf("   -b max bytes per object (default 1024)\n");
	printf("   -r ratio of get ops per put op (default 1)\n");
	printf("   -u make object keys unique to the HOST\n");
	printf("   -U make object keys unique to the entire cluster\n");
	printf("   -e expire time for objects (ttl) (default 0)\n");
	printf("\n   Default behaviour:\n");
	printf("       Simutanous PUT/GET/DEST Ops\n");
	printf("       Object sizes are random upto -b bytes\n");
	printf("       Object are left in the cache\n");
	printf("       Keys are shared cluster wide.   Keys are named: key:obj_number\n");
	printf("       -u keys shared HOST wide.       Keys are named: key:host:obj_number\n");
	printf("       -U keys UN-shared cluster wide. Keys are named: key:host:tid:obj_number\n");
	exit(0);
}

/* Logic to determine cache ops to perform */
void *cache_operations(void *opr)
{
	struct timespec interval, remainder; 
	interval.tv_sec = 0;
	interval.tv_nsec = 100000000;  /* 100000000 = 100ms */

	long tid;
	uint64_t verb=(uint64_t)opr;
	int loop_count=0;
	int put_err=0;
	int get_err=0; 
	int del_err=0;
	char key[1024];
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

					if ( verb == PUT )
						{
							int len, obj_count=0;									/* put loop */
							while ( obj_count < obj_max )
								{
									gen_key_name(key, host, tid, obj_count);
									/* if ( obj_count == 0 )							only gen a new object for obj_count 0 - save rand str time */
									len = gen_rand_str(big_obj);
									/* printf ("Object len: %d\n", len); */
									big_obj[0]= (char) obj_count;			/* replace first element with (char) of obj_count - uniqs each obj */ 

									if ( sleep_flag )
										nanosleep(&interval, &remainder);

									if (rna_put(big_obj, key, ttl, len, tid) == 0)		/* put object into cache */
										bytes_put+=(len - 1);
									else 
										put_err++;

									++obj_count;
									big_obj[0]='\0';                           	/* Reset obj buffer - set first val to null */
								}
						}

					if ( verb == GET )
						{
							int get_count=0;
							while ( get_count < put_get_ratio )			/* get each object  put_get_ratio times */
								{
									int obj_count=0;
									while ( obj_count < obj_max )
										{
											gen_key_name(key, host, tid, obj_count);
											if ( sleep_flag )
												nanosleep(&interval, &remainder);

											if ( rna_get(key, buffer, buffer_size, object_size, tid) != 0)
												++get_err;

											++get_count;
											++obj_count; /*increment obj count for next loop */
										}
								} /* end put_get_ratio */
						} /* end get */

					/* Object Destroy routine */
					if ( verb == DEL )
						{
							printf("Start destroy keys\n");
							int rc, obj_count=0;
							while ( obj_count < obj_max )
								{
									gen_key_name(key, host, tid, obj_count);
									rc = rna_destroy(key, tid);
									("tid %ld ", tid);
									if (rc != 0)
											del_err++;
									obj_count++;
								}
						}

					if ( clean_flag ) /* Post loop obj clean-up */
						{
							printf("Post loop key clean-up\n");
							int rc, obj_count=0;
							while ( obj_count < obj_max )
								{
									gen_key_name(key, host, tid, obj_count);
									rc = rna_destroy(key, tid);
									if (rc != 0)
											del_err++;
									obj_count++;
								}
						}

					++loop_count;
				} /* end loop_max */

	if ( verb == PUT )
		{
			printf("tid %ld TOTAL BYTES PUT: %llu\n", tid, bytes_put);
			printf("tid %ld PUT ERRORS: %d\n", tid, put_err);
		}
	if ( verb == GET )
		printf("tid %ld GET ERRORS: %d\n", tid, get_err);

	free(big_obj);
	free(buffer);
	pthread_exit(NULL);
}


/* put object with *key */
int rna_put(char *big_obj, char *key, int ttl, int len, long tid)
{
	int fail_flag=1;	/* Assume failure */
	int rc;
	rc = rna_share_put(key, big_obj, len, ttl);
	if (rc == 0)
		/* printf("Success PUT key: %s of size %d with ttl of %d with rc %d\n", key, len, ttl, rc); */
	fail_flag=0;
	else
		printf("Error PUT key: %s of size %d with ttl of %d with rc %d %ld\n", key, len, ttl, rc, tid);
	return rc;
}


/* Get object for *key */
int rna_get(char *key, void *buffer, uint64_t buffer_size, uint64_t object_size, long tid)
{
	int fail_flag=1;	/* Assume failure */
	int rc = rna_share_get(key, buffer, buffer_size, &object_size);

	if (rc == object_size) 
		fail_flag=0;
	else if (rc < 0)
		{
			fail_flag = rc;
			if (rc == -ENOENT) 
				fprintf(stderr, "get: Object %s does not exist %ld\n", key, tid);
					else if (rc == -EFBIG) 
						fprintf(stderr, "GET error, buffer too small for object %s of size %d rc %d %ld\n", key, object_size, rc, tid);
					else
						printf("Non specific GET error key: %s of size %d with rc %d %ld\n", key, object_size, rc, tid);
		}

	return fail_flag;
}


/* destroy object for *key */
int rna_destroy(char *key, long tid)
{
	int fail_flag=1;	/* Assume failure */
	int rc = rna_share_destroy(key);

	if (rc == 0)
		/* printf("Success DEL key: %s with rc %d\n", key, rc); */
		fail_flag=0;
	else
		printf("Error DEL key: %s with rc %d %ld\n", key, rc, tid);
	return rc;
}

void gen_key_name(char *key, char *host, long tid, int obj_count)
{
	if ( host_uniq )
		{
			sprintf(key, "key:%s:%d", host, obj_count); /* Create obj key with host name - key is priv to host */
		} 
	else if ( cluster_uniq )
		{
			sprintf(key, "key:%s:%ld:%d", host, tid, obj_count); /* Create obj key with host and tid name private, cluster wide */
		}
	else
		sprintf(key, "key:%d", obj_count); /* Create obj key obj count*/
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
