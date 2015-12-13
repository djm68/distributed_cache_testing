#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "/opt/rnanetworks/inc/pgd.h" 

int obj_max, value_size;
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
               char buf[value_size+1];
               char value[value_size];
               set_key(n, buf);
               rna_share_put(buf, value, value_size, -1);
        }
}
 
int main(int argc, char *argv[])
{
	int c;
	char *argval;
  value_size=4096;
  obj_max=10;
	while ((c = getopt(argc, argv, ":o:b:h")) != -1) {
			switch(c) {
					case 'h':
							printf("./create_obj -o num objects -b size of objects (bytes)\n");
							exit(0);
							break;
					case 'o':
							argval = optarg;
							obj_max = atoi(argval);
							break;
					case 'b':
							argval = optarg;
							value_size = atoi(argval);
							break;
					case ':':
							printf("-%c without arg\n", optopt);
							printf("./create_obj -o num objects -b size og objects (bytes)\n");
							exit(1);
							break;
					case '?':
							printf("./create_obj -o num objects -b size og objects (bytes)\n");
							printf("unknown arg %c\n", optopt);
							exit(1);
							break;
					}
		}

 	int rc = rna_share_open(NULL);
	int n;
 	if(rc != 0) {
 		fprintf(stderr, "COULD NOT CONNECT TO RNA: %d\n", rc);
		 return -1;
	 }


   create_keys();
}
