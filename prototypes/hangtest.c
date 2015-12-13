#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "/opt/rnanetworks/inc/pgd.h" 
 
#define VALUE_SIZE 4096

int obj_max, num_threads;
extern char *optarg;
extern int optind, optopt, opterr;
 
void set_key(int n, char* buf)
{
        sprintf(buf, "%d", n);
}
 
void create_keys()
{
        int n;
        for(n = 0; n != obj_max; ++n) {
               char buf[128];
               char value[VALUE_SIZE];
               set_key(n, buf);
               rna_share_put(buf, value, VALUE_SIZE, -1);
        }
}
 
void* get_keys(void* ptr)
{
				long tid;	
				tid = pthread_self();
        char* value_buf = (char*)malloc(VALUE_SIZE+1);
        uint64_t value_length = 0;
        int n;
        for(n = 0; n != obj_max; ++n) {
               int rc;
               char buf[128];
               set_key(n, buf);
               rc = rna_share_get(buf, value_buf, VALUE_SIZE, &value_length);
               if(rc < 0) {
                       fprintf(stderr, "ERROR: %d\n", rc);
               }
							else {
                       printf("tid %ld SUCCESS get obj %d with rc %d\n",tid,n,rc);
							}
        }
        free(value_buf); 
        return 0;
}
 
int main(int argc, char *argv[])
{
	int c;
	char *argval;
  obj_max=10;
  num_threads=1;

	while ((c = getopt(argc, argv, ":o:p:")) != -1) {
			switch(c) {
					case 'o':
							argval = optarg;
							obj_max = atoi(argval);
							break;
					case 'p':
							argval = optarg;
							num_threads = atoi(argval);
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
	for(n = 0; n != num_threads; ++n) {
		pthread_create(&threads[n], NULL, get_keys, NULL);
	}
 
	for(n = 0; n != num_threads; ++n) {
		pthread_join(threads[n], NULL);
	}

	exit (0);
}
