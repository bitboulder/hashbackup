PRG=hashbackup
SRCS=$(shell ls *.c)

CC=gcc
LFLAGS=-lssl -lcrypto -lz -lpthread
CFLAGS=-O4 -Wall -pedantic -std=c99

include make.inc

