CC = gcc
CFLAGS = -Wall -Wextra -g
LIBS = -lrt -pthread -lm

.PHONY: all clean clean_logger help run

all: dziekan kandydat komisja

dziekan: dziekan.c common.h
	$(CC) $(CFLAGS) -o dziekan dziekan.c $(LIBS)

kandydat: kandydat.c common.h
	$(CC) $(CFLAGS) -o kandydat kandydat.c $(LIBS)

komisja: komisja.c common.h
	$(CC) $(CFLAGS) -o komisja komisja.c $(LIBS)

clean:
	rm -f dziekan kandydat komisja *.txt

clean_logger:
	rm -rf logi

run: clean all
	./dziekan

help:
	@echo "Dostepne opcje:"
	@echo "  make all          - Kompiluje caly projekt"
	@echo "  make run          - Czysci binarki, kompiluje i uruchamia program"
	@echo "  make clean        - Usuwa pliki wykonywalne i raporty .txt (zostawia logi)"
	@echo "  make clean_logger - Usuwa katalog z logami"
	@echo "  make help         - Wyswietla te informacje"