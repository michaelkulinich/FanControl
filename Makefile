CC=g++
#CFLAGS=-I.
CFLAGS+= -Wall
LDFLAGS+= -lpthread

executables = serverFstd client 
all: $(executables)


DEPS = serverFstd.h Fan,h
OBJ = serverFstd.o Fan.o
serverFstd: $(OBJ)
%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $^ $< $(CFLAGS)

OBJ = client.o
client: $(OBJ)
	$(CC) -o $@ $< $(CFLAGS)

PHONY: clean
clean:
	rm -f serverFstd client $(wildcard *.o)
