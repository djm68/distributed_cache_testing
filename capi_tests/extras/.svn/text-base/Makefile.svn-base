CC=gcc
CFLAGS=/usr/lib64/librnashare.so

all: atomic_destroyer put_get_del put_get_reuse sha144

atomic_destroyer:
	$(CC) atomic_destroyer.c $(CFLAGS) -o atomic_destroyer

put_get_del:
	$(CC) put_get_del.c $(CFLAGS) -o put_get_del

put_get_reuse:
	$(CC) put_get_reuse.c $(CFLAGS) -o put_get_reuse

sha144:
	$(CC) sha144.c $(CFLAGS) -o sha144

clean:
	rm -f atomic_destroyer put_get_del put_get_reuse sha144
