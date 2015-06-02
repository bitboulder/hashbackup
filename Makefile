LFLAGS=-lssl -lcrypto -lz
CFLAGS=-g -Wall -pedantic -std=c99

hashbackup: $(shell ls *.c *.h)
	gcc -o hashbackup $(LFLAGS) $(CFLAGS) *.c
