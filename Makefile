CC = gcc
CFLAGS = -Wall -Wextra -g
# LDFLAGS dla konkretnych programów
LDFLAGS_DZIEKAN = -lrt
LDFLAGS_KOMISJA = -pthread

# Domyślny cel - kompiluje wszystko
all: dziekan kandydat komisja

# 1. Dziekan (korzysta z SHM i FIFO)
dziekan: dziekan.c common.h
	$(CC) $(CFLAGS) dziekan.c -o dziekan $(LDFLAGS_DZIEKAN)

# 2. Kandydat (korzysta z FIFO)
kandydat: kandydat.c common.h
	$(CC) $(CFLAGS) kandydat.c -o kandydat

# 3. Komisja (korzysta z wątków)
komisja: komisja.c common.h
	$(CC) $(CFLAGS) komisja.c -o komisja $(LDFLAGS_KOMISJA)

# Czyszczenie plików binarnych i logów
clean:
	rm -f dziekan kandydat komisja lista_przyjetych.txt lista_odrzuconych.txt

# Pomocniczy cel do testowania (uruchamia tylko dziekana, resztę uruchomi execl)
run: all
	./dziekan