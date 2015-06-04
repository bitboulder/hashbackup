LFLAGS=-lssl -lcrypto -lz -lpthread
CFLAGS=-O4 -Wall -pedantic -std=c99

all: hashbackup

hashbackup: $(shell ls *.c *.h)
	gcc -o hashbackup $(CFLAGS) *.c $(LFLAGS)

clean:
	rm hashbackup

install:
	install -D hashbackup $(DESTDIR)/usr/bin/hashbackup

uninstall:
	rm -f $(DESTDIR)/usr/bin/hashbackup

