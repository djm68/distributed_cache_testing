/******************************************************************************
 *                  Copyright (C) 2010 by RNA Networks, Inc.                  *
 *                         Author: Dominic Maraglia                           *
 *                      dominic.maraglia@rnanetworks.com                      *
 *                                                                            *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <inttypes.h>
#include "/opt/rnanetworks/inc/pgd.h"

// Function defs
void usage();
int del_key_obj(char *key);


int main(int argc, char *argv[])
{
// Globals to save pain of passing many cmd line args
  int c;
  char *key_4_obj;
  extern char *optarg;
  extern int optind, optopt, opterr;

  // Parse command line args
  while ((c = getopt(argc, argv, ":k:")) != -1) {
      switch(c) {
      case 'h':
          usage();
          break;
      case 'k':
          key_4_obj = optarg;
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
 
  // Initialize Object Caching interface
  int rc = rna_share_open(NULL);
  if (rc < 0) {
    printf("ERROR: failed to init RNA OCI\n");
    exit(rc);
  }
 
  rc = del_key_obj(key_4_obj);

  exit(rc);
}


void usage()
{
  printf("atomic_destroyer -k key val\n");
  printf("   -k key value for object to destroy\n");
  printf("   -h Help; this menu\n");
  exit(0);
}


// put, get, del cache objects
int del_key_obj( char *obj_key)
{
  int rc; 

  // Delete object cache
  rc = rna_share_destroy(obj_key);

  if (rc == 0)
    printf("SUCCESS DEL KEY: %s with rc %d\n", obj_key, rc);
  else
    printf("ERROR DEL KEY: %s with rc %d\n", obj_key, rc);

  return(rc);
}
