#include <assert.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
 
#include "pgd.h"
 
void set_key(int n, char* buf)
{
        sprintf(buf, "%d", n);
}
 
#define VALUE_SIZE (4096)
 
void create_keys()
{
        int n;
        for(n = 0; n != 100000; ++n) {
               char buf[128];
               char value[VALUE_SIZE];
               set_key(n, buf);
               rna_share_put(buf, value, VALUE_SIZE, -1);
        }
}
 
void* get_keys(void* ptr)
{
        char* value_buf = (char*)malloc(VALUE_SIZE);
        uint64_t value_length = 0;
        int n;
        for(n = 0; n != 10000; ++n) {
               int rc;
               char buf[128];
               set_key(n%100, buf);
               rc = rna_share_get(buf, value_buf, VALUE_SIZE, &value_length);
               if(rc < 0) {
                       fprintf(stderr, "ERROR: %d\n", rc);
               }
 
               assert(value_length == VALUE_SIZE);
        }
 
        fprintf(stderr, "DONE GET KEYS\n");
        return 0;
}
 
int main(int argc, char** argv)
{
        int NUM_THREADS = 1;
        int num_iterations = 1;
 
        if(argc > 1) {
               NUM_THREADS = atoi(argv[1]);
               if(argc > 2) {
                       num_iterations = atoi(argv[2]);
               }
        }
 
        fprintf(stderr, "RUNNING WITH %d THREADS, %d ITERATIONS\n", NUM_THREADS, num_iterations);
 
        pthread_t threads[NUM_THREADS];
        int rc = rna_share_open(NULL);
        int n;
        if(rc != 0) {
               fprintf(stderr, "COULD NOT CONNECT TO RNA: %d\n", rc);
               return -1;
        }
 
        for(n = 0; n != num_iterations; ++n) {
               create_keys();
        }
 
        while(num_iterations--) {
               for(n = 0; n != NUM_THREADS; ++n) {
                       pthread_create(&threads[n], NULL, get_keys, NULL);
               }
 
               for(n = 0; n != NUM_THREADS; ++n) {
                       pthread_join(threads[n], NULL);
               }
        }
}
