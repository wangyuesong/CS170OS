CC = gcc

EXECUTABLES = source

CFLAGS = -g

all: $(EXECUTABLES)

clean:
	rm -f core *.o $(EXECUTABLES) a.out *a

source.o: source.c
	$(CC) $(CFLAGS) -c source.c
