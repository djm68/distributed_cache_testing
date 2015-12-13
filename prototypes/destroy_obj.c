#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "/opt/rnanetworks/inc/pgd.h" 

int obj_max;
extern char *optarg;
extern int optind, optopt, opterr;
 
void destroy_keys()
{
				char buf[1024];
        int n;
        for(n = 0; n != obj_max; ++n) {
							 sprintf(buf, "%d", n);
               rna_share_destroy(buf);
        }
}


int main(int argc, char *argv[])
{
	int c;
	char *argval;
  obj_max=10;
	while ((c = getopt(argc, argv, ":o:h")) != -1) {
			switch(c) {
					case 'h':
							printf("./destroy_obj -o num objects\n");
							exit(0);
							break;
					case 'o':
							argval = optarg;
							obj_max = atoi(argval);
							break;
					case ':':
							printf("-%c without arg\n", optopt);
							printf("./destroy_obj -o num objects\n");
							exit(1);
							break;
					case '?':
							printf("./destroy_obj -o num objects\n");
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

  destroy_keys();
}
