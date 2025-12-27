#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#define FIFO_WEJSCIE "kolejka_przed_wydzialem"
#define LICZBA_KANDYDATOW 120 // testowa wartosc 120 --  docelowo 1200 
#define SZANSA_NA_BRAK_MATURY 2 // 2% szansy na brak matury u kandydata

typedef struct {
    pid_t id;
    int zdana_matura; // 1 = tak, 0 = nie
} Zgloszenie;

#endif