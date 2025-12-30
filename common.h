#ifndef COMMON_H
#define COMMON_H

// Odblokowanie definicji POSIX (dla barier i nanosleep)
#define _XOPEN_SOURCE 700 
#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>

#define GODZINA_T 3         
#define GODZINA_Ti 500

#define MAX_KANDYDATOW 1500
#define LICZBA_KANDYDATOW 200 
#define SZANSA_NA_BRAK_MATURY 2 
#define SZANSA_NA_ZDANA_TEORIE 2 

#define FIFO_WEJSCIE "kolejka_przed_wydzialem"
#define SHM_NAME "/egzamin_shm_v1"

#define KOLEJKA_KOMISJA_A "/sem_kolejka_a" 
#define KOLEJKA_KOMISJA_B "/sem_kolejka_b" 
#define WOLNE_MIEJSCA_KOMISJA_A "/sem_miejsca_a" 
#define WOLNE_MIEJSCA_KOMISJA_B "/sem_miejsca_b" 

static inline void sleep_ms(int ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

typedef struct {
    pid_t id;
    int zdana_matura;        
    int zdana_teoria_wczesniej; 
} Zgloszenie;

typedef struct {
    pid_t id;
    int zdana_matura;
    int zdana_teoria_wczesniej;
    int punkty_teoria;   
    int punkty_praktyka; 
    int status;          
    int czy_przyjety;    

    int pytania[5];             
    int id_egzaminatora[5];     
    int odpowiedzi[5];          
    int oceny[5];               
    
    int status_arkusza; 
    
    int zaliczona_A; 
    int zaliczona_B; 

    pthread_mutex_t mutex_ipc;
    pthread_cond_t  cond_ipc;
} Student;

typedef struct {
    Student lista[MAX_KANDYDATOW];
    int liczba_kandydatow;
} EgzaminPamiecDzielona;

#define MAX_MIEJSC 3 
#define LICZBA_EGZAMINATOROW_A 5
#define LICZBA_EGZAMINATOROW_B 3
#define LICZBA_KOMISJI 2

typedef struct {
    int id_kandydata;              
    int pytania[5];                
    int liczba_zadanych_pytan;     
    int zajete;                    
    
    int potrzebni_czlonkowie; 

    pthread_mutex_t mutex;         
    pthread_barrier_t bariera;     
} Stanowisko;

#endif