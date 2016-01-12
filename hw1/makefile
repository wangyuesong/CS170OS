CC = gcc

EXECUTABLES = multi_thread matrix_generator one_thread

CFLAGS = -g

all: $(EXECUTABLES)

clean:
	rm -f core *.o $(EXECUTABLES) a.out *a

multi_thread.o: multi_thread.c
	$(CC) $(CFLAGS) -c multi_thread.c

one_thread.o: one_thread.c
	$(CC) $(CFLAGS) -c one_thread.c

matrix_generator.o: matrix_generator.c
	$(CC) $(CFLAGS) -c matrix_generator.c
