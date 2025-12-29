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
#include <pthread.h>

// --- Kandydaci -> Dziekan --- 
#define GODZINA_T 3 // czas jaki mija do godziny T (sekundy)
#define GODZINA_Ti 500000 // czas po ktorym kandydat zwraca odpowiedzi na pytania (msekundy)
#define MAX_KANDYDATOW 1500
#define FIFO_WEJSCIE "kolejka_przed_wydzialem"
#define LICZBA_KANDYDATOW 30 // Zmienione na 30 dla bezpiecznych testów
#define SZANSA_NA_BRAK_MATURY 2 
#define SZANSA_NA_ZDANA_TEORIE 2 

typedef struct {
    pid_t id;
    int zdana_matura;        
    int zdana_teoria_wczesniej; 
} Zgloszenie;

// --- Dziekan -> Dziekan ---
#define SHM_NAME "/egzamin_shm_v1"

typedef struct {
    pid_t id;
    int zdana_matura;
    int zdana_teoria_wczesniej;
    int punkty_teoria;   
    int punkty_praktyka; 
    int status;          
    int czy_przyjety;    
} Student;

// --- Komisja -> Dziekan ---
#define MAX_MIEJSC 3 

typedef struct {
    int id_kandydata;              
    int pytania[5];                
    int liczba_zadanych_pytan;     
    int suma_ocen;                 
    int zajete;                    
    int gotowe; // 0 = Przewodniczący wpisuje dane, 1 = Można pytać

    int kto_pytal[5]; 

    pthread_mutex_t mutex;         
    pthread_cond_t cond_bariera;   
} Stanowisko;

#define LICZBA_EGZAMINATOROW_A 5
#define LICZBA_EGZAMINATOROW_B 3
#define LICZBA_KOMISJI 2

#define KOLEJKA_KOMISJA_A "/sem_kolejka_a" 
#define KOLEJKA_KOMISJA_B "/sem_kolejka_b" 

#define WOLNE_MIEJSCA_KOMISJA_A "/sem_miejsca_a" 
#define WOLNE_MIEJSCA_KOMISJA_B "/sem_miejsca_b" 

// Pamięć dzielona
typedef struct {
    Student lista[MAX_KANDYDATOW];
    int liczba_kandydatow;
} EgzaminPamiecDzielona;

#endif