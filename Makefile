LFLAGS=-lssl -lcrypto -lz
CFLAGS=-g -Wall -pedantic -std=c99

all: hashbackup

hashbackup: $(shell ls *.c *.h)
	gcc -o hashbackup $(LFLAGS) $(CFLAGS) *.c

clean:
	rm hashbackup

install:
	install -D hashbackup $(DESTDIR)/usr/bin/hashbackup

uninstall:
	rm -f $(DESTDIR)/usr/bin/hashbackup

