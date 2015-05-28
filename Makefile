LFLAGS=-lssl -lcrypto -lz
CFLAGS=-g -Wall -pedantic -std=c99

backdat: $(shell ls *.c *.h)
	gcc -o backdat $(LFLAGS) $(CFLAGS) *.c
