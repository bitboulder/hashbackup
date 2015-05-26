LFLAGS=
CFLAGS=-g -Wall -pedantic

backdat: $(shell ls *.c *.h)
	gcc -o backdat $(LFLAGS) $(CFLAGS) *.c
