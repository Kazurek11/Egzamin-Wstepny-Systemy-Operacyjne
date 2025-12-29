CC = gcc
CFLAGS = -Wall -Wextra -g

# Biblioteki konieczne do linkowania:
# -lrt:     wymagane przez shm_open, shm_unlink, mmap
# -pthread: wymagane przez wątki, mutexy ORAZ semafory
LIBS = -lrt -pthread

# Domyślny cel - kompiluje wszystko
all: dziekan kandydat komisja

# 1. Dziekan
dziekan: dziekan.c common.h
	$(CC) $(CFLAGS) dziekan.c -o dziekan $(LIBS)

# 2. Kandydat (tu wystarczy samo CFLAGS, ale LIBS nie zaszkodzi)
kandydat: kandydat.c common.h
	$(CC) $(CFLAGS) kandydat.c -o kandydat

# 3. Komisja
komisja: komisja.c common.h
	$(CC) $(CFLAGS) komisja.c -o komisja $(LIBS)

# Czyszczenie plików binarnych i logów
clean:
	rm -f dziekan kandydat komisja lista_przyjetych.txt lista_odrzuconych.txt

# Uruchamianie (najpierw czyści, potem kompiluje, potem uruchamia)
run: clean all
	./dziekan