#ifndef COMMON_H
#define COMMON_H

#define _XOPEN_SOURCE 700 
#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h> 
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h> 
#include <semaphore.h>    
#include <signal.h> 
#include <string.h>      
#include <stdarg.h>

#define GODZINA_T 2         
#define GODZINA_Ti 200

#define MAX_KANDYDATOW 2000
#define LICZBA_KANDYDATOW 1800
#define CHETNI_NA_MIEJSCE 10
#define LIMIT_PRZYJEC (LICZBA_KANDYDATOW / CHETNI_NA_MIEJSCE)
#define SZANSA_NA_BRAK_MATURY 2
#define SZANSA_NA_ZDANA_TEORIE 2

#define FIFO_WEJSCIE "kolejka_przed_wydzialem"
#define SHM_NAME "/egzamin_shm_final_v3"

#define KOLEJKA_KOMISJA_A "/sem_kolejka_a" 
#define KOLEJKA_KOMISJA_B "/sem_kolejka_b" 
#define WOLNE_MIEJSCA_KOMISJA_A "/sem_miejsca_a" 
#define WOLNE_MIEJSCA_KOMISJA_B "/sem_miejsca_b" 

#define SEM_LOG_KEY "/sem_logger_synchronizacja"
#define SEM_SYNC_START "/sem_sync_kolejnosc"

#define SEM_LICZNIK_KONCA "/sem_licznik_zakonczonych_spraw"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

static char *AKTUALNY_KOLOR_LOGU = ANSI_COLOR_RESET;

static inline void sleep_ms(int ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

static inline void utworz_folder(const char *sciezka) {
    struct stat st = {0};
    if (stat(sciezka, &st) == -1) {
        mkdir(sciezka, 0777);
    }
}

static inline void pobierz_czas(char *buffer, size_t size) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *tm_info = localtime(&tv.tv_sec);
    int millis = tv.tv_usec / 1000;
    snprintf(buffer, size, "%02d:%02d:%02d.%03d", 
             tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, millis);
}

static inline FILE* otworz_log(const char *nazwa_bazowa, const char *tryb) {
    utworz_folder("logi");
    char nazwa_pelna[256];
    if (strncmp(nazwa_bazowa, "logi/", 5) == 0) {
        snprintf(nazwa_pelna, sizeof(nazwa_pelna), "%s", nazwa_bazowa);
    } else {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        snprintf(nazwa_pelna, sizeof(nazwa_pelna), "logi/%04d-%02d-%02d_%02d-%02d-%02d_logi_%s.txt",
                 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                 tm.tm_hour, tm.tm_min, tm.tm_sec, nazwa_bazowa);
    }
    FILE *fp = fopen(nazwa_pelna, tryb);
    if (!fp) {
        perror("Błąd otwierania logu");
        return NULL;
    }
    return fp;
}

static inline void dodaj_do_loggera(FILE *plik, const char *format, ...) {
    char czas[32];
    pobierz_czas(czas, sizeof(czas));
    va_list args;
    
    // --- WYPISANIE NA EKRAN (KOLOROWE) ---
    printf("%s[Czas: %s]\t", AKTUALNY_KOLOR_LOGU, czas);
    
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf("%s", ANSI_COLOR_RESET);
    fflush(stdout); 

    // --- WYPISANIE DO PLIKU (CZYSTE, BEZ KOLORÓW) ---
    if (plik) {
        fprintf(plik, "[Czas: %s]\t", czas);
        va_start(args, format); 
        vfprintf(plik, format, args);
        va_end(args);
        fflush(plik); 
    }
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
    double punkty_teoria;   
    double punkty_praktyka; 
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
    
    pthread_mutex_t mutex_rejestracji;
    pthread_cond_t  cond_rejestracji;
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