CC=gcc
CFLAGS=/usr/lib64/librnashare.so
LDFLAGS=-lpthread

all: create_obj destroy_obj time_get

create_obj:
	$(CC) create_obj.c $(CFLAGS) -o create_obj

destroy_obj:
	$(CC) destroy_obj.c $(CFLAGS) -o destroy_obj

time_get:
	$(CC) time_get.c $(CFLAGS) $(LDFLAGS) -o time_get

clean:
	rm -f create_obj time_get destroy_obj
