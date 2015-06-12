PRG=hashbackup
SRCS=$(shell ls *.c)

CC=gcc
LFLAGS+=-lssl -lcrypto -lz -lpthread
CFLAGS+=`pkg-config --cflags ext2fs`
LFLAGS+=`pkg-config --libs ext2fs`

include make.inc

