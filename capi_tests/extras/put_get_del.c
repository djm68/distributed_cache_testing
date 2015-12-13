/******************************************************************************
 *                  Copyright (C) 2010 by RNA Networks, Inc.                  *
 *                         Author: Dominic Maraglia                           *
 *                      dominic.maraglia@rnanetworks.com                      *
 *                                                                            *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h> 
#include <unistd.h> 
#include <inttypes.h>
#include <sys/types.h>
#include "/opt/rnanetworks/inc/pgd.h"
#define MIN_SIZE 20

// Function defs
void usage();
int cache_operations();
void gen_rand_str(char *dst, int size);

// Globals to save pain of passing many cmd line args
int c, norand_flag, get_flag, del_flag, obj_max, bytes_max, loop_max, ttl;
char *objs, *bytes, *loops, *ttls;
extern char *optarg;
extern int optind, optopt, opterr;

int main(int argc, char *argv[])
{

  // Set some defaults
  ttl=0;          // Time to live for obj; 0 = forever
  obj_max=10;     // num of keys to put/get/del
  loop_max=1;     // num of times to loop put/get/del
  bytes_max=100;  // max size of object to create

  // Parse command line args
  while ((c = getopt(argc, argv, ":Rhgdo:b:l:t:")) != -1) {
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
      case 'o':
          objs = optarg;
          obj_max = atoi(objs);
          break;
      case 'b':
          bytes = optarg;
          bytes_max = atoi(bytes);
          break;
      case 'l':
          loops = optarg;
          loop_max = atoi(loops);
          break;
      case 't':
          ttls = optarg;
          ttl = atoi(ttls);
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
 
  // Initialize rand seed
  srand(time(0));

  // Initialize Object Caching interface
  int rc = rna_share_open(NULL);
  if (rc < 0) {
    printf("ERROR: failed to init RNA OCI\n");
    exit(rc);
  }

  // go do the real work
  rc=cache_operations();

  exit(rc);
}

void usage()
{
  printf("put_get_del -h -g -d -R -o num -l num -b num -t num\n");
  printf("   -g flag get objects after put\n");
  printf("   -d flag destroy objects after all puts/gets\n");
  printf("   -o num objects to create per loop\n");
  printf("   -l num of loops i.e.: put/get object n number of times\n");
  printf("   -b max bytes per object\n");
  printf("   -t time to live for and object\n");
  printf("   -R disable random object size; objects will be exactly -b num bytes\n");
  printf("   -h Help; this menu\n");
  exit(0);
}

// put, get, del cache objects
int cache_operations()
{
  long long unsigned int byte_count=0;
  int obj_len, rc; 
  int loop_count=0;
  int put_err=0;
  int get_err=0; 
  int del_err=0;
  int cmp_err=0;
  char key[1024];
  char host[1024];
  pid_t pid;

  // get pid and hostname for obj key
  pid=getpid();
  host[1023] = '\0';
  gethostname(host, 1023);

  // Allocate memory for buffer
  void *buffer;
  uint64_t object_size;
  uint64_t buffer_size = bytes_max+1;
  buffer = malloc(buffer_size);
  if (buffer == NULL)
    {
      printf("Buffer memory not allocated.\n");
      exit(1);
    }

  // Allocate memory for object
  char *big_obj = (char *)malloc(buffer_size);
  if (big_obj == NULL)
    {
      printf("Object memory not allocated.\n");
      exit(1);
    }

  while ( loop_count < loop_max )
    {
      int obj_count=0;      
      loop_count++;
      printf("Start loop %d of %d\n", loop_count, loop_max);
      while ( obj_count < obj_max )
        {
          obj_count++;
          // Create obj key "key:" + pid + obj_count
          sprintf(key, "key_%s_%d_%d", host, pid, obj_count);

          // Rand size object or fixed?
          if (norand_flag)
            obj_len = bytes_max;                 // fixed object size
          else
            obj_len = ( rand() % (bytes_max) );  // rand object size
          
          if ( obj_len < MIN_SIZE )             // make sure object size in never < MIN_SIZE
            obj_len = MIN_SIZE;

          // Generate object of obj_len bytes
          gen_rand_str(big_obj, obj_len);

          // Insert object into cache, +1 for '\0'
          printf("LOOP:%d ", loop_count);
          rc = rna_share_put(key, big_obj, strlen(big_obj)+1, ttl);
          if (rc == 0)
            {
              printf("SUCCESS PUT KEY: %s of size %d with rc %d\n", key, strlen(big_obj)+1, rc);
              byte_count = byte_count + obj_len;  // Increment byte count upon success
            }
          else
            {
              printf("ERROR PUT KEY: %s of size %d with rc %d\n", key, strlen(big_obj)+1, rc);
              put_err++;
            }

          // Get object from cache?
          if (get_flag)
            {
              printf("LOOP:%d ", loop_count);
              rc = rna_share_get(key, buffer, buffer_size, &object_size);
              if (rc == strlen(big_obj)+1)
                printf("SUCCESS GET KEY: %s of size %d with rc %d\n", key, strlen(big_obj)+1, rc);
              else
                {
                  printf("ERROR GET KEY: %s of size %d with rc %d\n", key, strlen(big_obj)+1, rc);
                  get_err++;
                }

              // Does GET value agree w/original generated object
              printf("LOOP:%d ", loop_count);
              if (strcmp(big_obj, buffer) == 0 )
                printf("POS_MATCH KEY: %s\n", key);
              else
                {
                  printf("NEG_MATCH KEY: %s\n", key);
                  cmp_err++;
                }
 
              // Only print objects if they are < 80 bytes 
              if (strlen(big_obj) <= 80)
                  printf("LOOP:%d   ORIG_VAL: %s  RNA_BUFF: %s\n", loop_count, big_obj, buffer);
            }
        } // end obj_max

        // Delete objects from cache?
        if (del_flag)
          {
            int obj_count=0;
            while ( obj_count < obj_max )
              {
                obj_count++;
                sprintf(key, "key_%s_%d_%d", host, pid, obj_count);
                rc = rna_share_destroy(key);

                if (rc == 0)
                  printf("SUCCESS DEL KEY: %s with rc %d\n", key, rc);
                else
                  {
                    printf("ERROR DEL KEY: %s with rc %d\n", key, rc);
                    del_err++;
                  }
              }
          }

  } // end loop_max

  printf("TOTAL BYTES PUT: %llu\n", byte_count);
  printf("ERRORS  PUT: %d  GET: %d  DEL: %d  COMP: %d\n",put_err, get_err, del_err, cmp_err);
  free(big_obj);
  free(buffer);
  return(put_err+get_err+del_err+cmp_err);
}


// Gen a rand string of size in bytes
void gen_rand_str(char *dst, int size)
{
  int i;
  static const char text[] = "abcdefghijklmnopqrstuvwxyz0123456789"
                             "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  for ( i = 0; i < size; ++i )
  {
    dst[i] = text[rand() % (sizeof text - 1)];
  }
  dst[i] = '\0';
  return dst;
}
