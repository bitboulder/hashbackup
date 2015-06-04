PRG=hashbackup
SRCS=$(shell ls *.c)

CC=gcc
LFLAGS+=-lssl -lcrypto -lz -lpthread

include make.inc

