CC=gcc
CFLAGS=-g -Wall -pthread

SRCDIR=src
INCDIR=include
LIBDIR=lib

all: outdir $(LIBDIR)/utils.o server client

server: $(LIBDIR)/utils.o $(INCDIR)/utils.h $(SRCDIR)/server.c
	$(CC) $(CFLAGS) -I$(INCDIR) -o $@ $(LIBDIR)/utils.o $(SRCDIR)/server.c -lm

client: $(LIBDIR)/utils.o $(INCDIR)/utils.h $(SRCDIR)/client.c
	$(CC) $(CFLAGS) -I$(INCDIR) -o $@ $(LIBDIR)/utils.o $(SRCDIR)/client.c -lm
	mkdir -p output/img
	
.PHONY: clean outdir

clean:
	rm -f server client
	rm -rf output
	rm -f output/img/*

zip:
	@make clean
	zip -r project-3.zip include lib src database expected Makefile README.md
