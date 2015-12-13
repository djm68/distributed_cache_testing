CC=gcc
CFLAGS=/usr/lib64/librnashare.so
LDFLAGS=-lpthread

all: atomizer md5_mget time_mget verbs_ser verbs_para

atomizer:
	$(CC) atomizer.c $(CFLAGS) -o atomizer

time_mget:
	$(CC) time_mget.c $(CFLAGS) $(LDFLAGS) -lcrypto -lssl -o time_mget

md5_mget:
	$(CC) md5_mget.c $(CFLAGS) $(LDFLAGS) -lcrypto -lssl -o md5_mget

verbs_ser:
	$(CC) verbs_ser.c $(CFLAGS) $(LDFLAGS) -o verbs_ser

verbs_para:
	$(CC) verbs_para.c $(CFLAGS) $(LDFLAGS) -o verbs_para

clean:
	rm -f atomizer md5_mget time_mget verbs_ser verbs_para
