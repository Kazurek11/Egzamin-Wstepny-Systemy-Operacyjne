CC = gcc
CFLAGS = -Wall -Wextra -g
LIBS = -lrt -pthread -lm

all: dziekan kandydat komisja

dziekan: dziekan.c common.h
	$(CC) $(CFLAGS) -o dziekan dziekan.c $(LIBS)

kandydat: kandydat.c common.h
	$(CC) $(CFLAGS) -o kandydat kandydat.c $(LIBS)

komisja: komisja.c common.h
	$(CC) $(CFLAGS) -o komisja komisja.c $(LIBS)

clean:
	rm -f dziekan kandydat komisja *.txt

run: clean all
	./dziekan