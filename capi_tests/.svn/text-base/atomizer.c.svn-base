/******************************************************************************
 *                  Copyright (C) 2010 by RNA Networks, Inc.                  *
 *                         Author: Dominic Maraglia                           *
 *                      dominic.maraglia@rnanetworks.com                      *
 *                                                                            *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h> 
#include <inttypes.h>
#include <sys/types.h>
#include "/opt/rnanetworks/inc/pgd.h"

/* Function defs */
void help();
void usage();
void interactive(int bytes);
void rna_destroy( char *key);
void rna_put(char *big_obj, char *key, int ttl, int bytes);
void rna_get(char *big_obj, char *key, void *buffer, uint64_t buffer_size, uint64_t object_size);
void gen_rand_str(char *dst, int size);

int main(int argc, char *argv[])
{
  int c, bytes_max = 1024;;
  extern char *optarg;
  extern int optind, optopt, opterr;

  while ((c = getopt(argc, argv, ":m:")) != -1)  /* Parse cmd line opts */
    { 
      switch(c) 
        {
          case 'h':
               usage();
               break;
          case 'm':
               bytes_max = atoi(optarg);
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

  int rc = rna_share_open(NULL);                 /* Init OCI */
  if (rc < 0) 
    {
      printf("Error: failed to init RNAShare OCI\n");
      exit(rc);
    }
  else 
      printf("RNAShare OCI initialized successfully -- Ready to accept commands.\n");

  help();                                        /* print cmd summary */
  interactive(bytes_max);
  exit(0);
}

void usage()
{
  printf("atomizer -m bytes\n");
  printf("   -m Max object size(bytes); default 1024\n");
  printf("   -h Help; this menu\n");
  exit(0);
}

// Shell like cli for put/get/destroy
void interactive(int bytes_max)
{
  char op[2], key[40];
  char *line = NULL;
  size_t len=0;
  ssize_t read;
  int ttl, bytes;

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

  while ((read = getline(&line, &len, stdin)) != -1) 
    {
      // zero out op and key
      memset(op, '\0', sizeof(op));
      memset(key, '\0', sizeof(key));
      ttl=0;                  // default ttyl
      bytes=128;              // default obj size

      sscanf(line, "%s %s", op, key);
        // printf("2 %s %s\n", op, key);
      sscanf(line, "%s %s %d", op, key, &ttl);
        // printf("3 %s %s %d\n", op, key, ttl);
      sscanf(line, "%s %s %d %d", op, key, &ttl, &bytes);
        // printf("4 %s %s %d\n", op, key, ttl, bytes);

      // handle cases without key
      if (!key[0])
        switch(op[0]) // switch on first char of *op
        {
          case 'q':
              printf("\nExiting RNAShare OCI Shell...\n");
              free(line);
              free(buffer);
              free(big_obj);
              exit(0);
              break;
          case 'h':
              help(); 
              continue;
          default:
              printf("Invalid command\n> "); 
              continue;
        }

      switch(op[0]) // switch on first char of *op
        {
              help();
              break;
          case 'q':
              printf("\nExiting RNAShare OCI Shell...\n");
              free(line);
              free(buffer);
              free(big_obj);
              exit(0);
              break;
          case 'd':
              rna_destroy(key);
              break;
          case 'g':
              rna_get(big_obj, key, buffer, buffer_size, object_size);
              break;
          case 'p':
              if ( bytes > bytes_max)
                bytes = bytes_max;
              rna_put(big_obj, key, ttl, bytes);
              break;
          default:
              printf("Invalid command\n"); 
              break;
        }
        printf("> ");
    }
    free(line);
    free(buffer);
    free(big_obj);
    return;
}

void help()
{
  printf("\nCommand Summary\n");
  printf("p key_name ttl bytes = put key_name with ttl(secs) and object of size bytes\n");
  printf("  ttl, bytes are optional, but are positional params; ttl must preceed bytes\n"); 
  printf("g key_name = get key_name from cache\n");
  printf("d key_name = destroy key_name from cache\n");
  printf("h = this menu\n");
  printf("q = quit\n\n> ");
  return;
}

/* put object with *key */
void rna_put(char *big_obj, char *key, int ttl, int obj_len)
{
  int rc;
  /* Generate object of obj_len bytes */
  gen_rand_str(big_obj, obj_len);

  /* Insert object into cache, +1 for '\0' */
  rc = rna_share_put(key, big_obj, strlen(big_obj)+1, ttl);
  if (rc == 0)
      printf("Success PUT key: %s of size %d with ttl of %d with rc %d\n", key, strlen(big_obj)+1, ttl, rc);
  else
      printf("Error PUT key: %s of size %d with ttl of %d with rc %d\n", key, strlen(big_obj)+1, ttl, rc);
   
  return;
} 


/* get object for *key */
void rna_get(char *big_obj, char *key, void *buffer, uint64_t buffer_size, uint64_t object_size)
{
  int rc = rna_share_get(key, buffer, buffer_size, &object_size);
  if (rc == strlen(big_obj)+1)
    printf("Success GET key: %s of size %d with rc %d\n", key, strlen(big_obj)+1, rc);
  else
      printf("Error GET key: %s of size %d with rc %d\n", key, strlen(big_obj)+1, rc);

  return;
}


/* destroy object for *key */
void rna_destroy(char *key)
{
  int rc = rna_share_destroy(key);

  if (rc == 0)
    printf("Success DEL key: %s with rc %d\n", key, rc);
  else
    printf("Error DEL key: %s with rc %d\n", key, rc);

  return;
}

/* Gen a rand string of size in bytes */
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
}
