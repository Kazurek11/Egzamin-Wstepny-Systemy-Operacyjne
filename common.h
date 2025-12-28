#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

// --- Kandydaci -> Dziekan --- 
#define MAX_KANDYDATOW 1500
#define FIFO_WEJSCIE "kolejka_przed_wydzialem"
#define LICZBA_KANDYDATOW 1200 // testowa wartosc 120 --  docelowo 1200 
#define SZANSA_NA_BRAK_MATURY 2 // 2% szansy na brak matury u kandydata
#define SZANSA_NA_ZDANA_TEORIE 2 // 2% szansy na zdana teorie w poprzednich latach

typedef struct {
    pid_t id;
    int zdana_matura;        // 1 = tak, 0 = nie
    int zdana_teoria_wczesniej; // 1 = tak (powtarza rok), 0 = nie (nowy)
} Zgloszenie;

// --- Dziekan -> Dziekan ---
#define SHM_NAME "/egzamin_shm_v1"

typedef struct {
    pid_t id;
    int zdana_matura;
    int zdana_teoria_wczesniej;
    
    // Pola ktore wypelni Komisja:
    int punkty_teoria;   // -1 oznacza "jeszcze nie zdawał"
    int punkty_praktyka; // -1 oznacza "jeszcze nie zdawał"
    int status;          // 0=Czeka, 1=Teoria, 2=Praktyka, 3=Koniec
    int czy_przyjety;    // 0 lub 1 (TO AKURAT WYPELNI DZIEKAN)
} Student;

// --- Komisja -> Dziekan ---
#define LICZBA_EGZAMINATOROW_A 5
#define LICZBA_EGZAMINATOROW_B 3
#define LICZBA_KOMISJI 2

#define KOLEJKA_KOMISJA_A "/sem_kolejka_a" // Kolejka osob do komisji A która bedzie kontrolował dziekan
#define KOLEJKA_KOMISJA_B "/sem_kolejka_b" // Kolejka osób do komisji B która bedzie kontrolowałą komisja A oraz dziekan w przypadku wczesniej zdanej teorii przez kandydata

#define WOLNE_MIEJSCA_KOMISJA_A "/sem_miejsca_a" // Wolne miejsca zwalniane przez komisje A po przepytaniu kandydata
#define WOLNE_MIEJSCA_KOMISJA_B "/sem_miejsca_b" // Wolne miejsca zwalniane przez komisje B po przepytaniu kandydata



// Pamięć dzielona
typedef struct {
    Student lista[MAX_KANDYDATOW];
    int liczba_kandydatow;
} EgzaminPamiecDzielona;

#endif