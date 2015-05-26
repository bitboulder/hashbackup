LFLAGS=
CFLAGS=-g -O2 -Wall -pedantic

backdat: $(shell ls *.c *.h)
	gcc -o backdat $(LFLAGS) $(CFLAGS) *.c
