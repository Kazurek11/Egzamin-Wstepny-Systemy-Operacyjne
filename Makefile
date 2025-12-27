all: dziekan kandydat

dziekan: dziekan.c common.h
	gcc -Wall dziekan.c -o dziekan -lrt

kandydat: kandydat.c common.h
	gcc -Wall kandydat.c -o kandydat

clean:
	rm -f dziekan kandydat