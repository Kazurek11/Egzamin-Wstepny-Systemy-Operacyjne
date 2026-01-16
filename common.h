#ifndef COMMON_H
#define COMMON_H

// Wymagane flagi kompilatora dla standardu POSIX (obsługa wątków, czasu, IPC)
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

// --- KONFIGURACJA SYMULACJI ---
#define GODZINA_T 5         // Czas oczekiwania na rozpoczęcie egzaminu (w sekundach)
#define GODZINA_Ti 300      // Jednostka czasu symulacji (np. czas pisania odpowiedzi)

// --- PARAMETRY REKRUTACJI ---
#define MAX_KANDYDATOW 1000 // Maksymalna pojemność systemu
#define LICZBA_KANDYDATOW 1000 // Ilu kandydatów faktycznie generujemy
#define CHETNI_NA_MIEJSCE 10   // Współczynnik trudności dostania się
#define LIMIT_PRZYJEC 1500     // Limit miejsc na roku (może być wyliczany lub stały)

// Szanse (w procentach) na posiadanie określonych cech przez kandydata
#define SZANSA_NA_BRAK_MATURY 20
#define SZANSA_NA_ZDANA_TEORIE 2

// --- KOMUNIKACJA MIĘDZYPROCESOWA (IPC) ---
#define FIFO_WEJSCIE "kolejka_przed_wydzialem" // Kolejka, gdzie kandydaci wrzucają zgłoszenia
#define SHM_NAME "/egzamin_shm_final_v3"       // Nazwa obiektu pamięci dzielonej (wspólny arkusz)

// Semafory sterujące przepływem ludzi
#define KOLEJKA_KOMISJA_A "/sem_kolejka_a"     // Semfor zliczający oczekujących do A
#define KOLEJKA_KOMISJA_B "/sem_kolejka_b"     // Semfor zliczający oczekujących do B
#define WOLNE_MIEJSCA_KOMISJA_A "/sem_miejsca_a" // Ile jest wolnych stolików w sali A
#define WOLNE_MIEJSCA_KOMISJA_B "/sem_miejsca_b" // Ile jest wolnych stolików w sali B

// Semafory synchronizacyjne
#define SEM_SYNC_START "/sem_sync_kolejnosc"     // Synchronizacja startu procesów
#define SEM_LICZNIK_KONCA "/sem_licznik_zakonczonych_spraw" // Licznik obsłużonych studentów (dla Dziekana)

// --- KOLORY DO LOGÓW W TERMINALU ---
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

static char *AKTUALNY_KOLOR_LOGU = ANSI_COLOR_RESET;

// --- FUNKCJE POMOCNICZE (INLINE) ---

// Usypia wątek na podaną liczbę milisekund (precyzyjniejsze niż sleep)
static inline void sleep_ms(int ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

// Tworzy folder na logi, jeśli nie istnieje
static inline void utworz_folder(const char *sciezka) {
    struct stat st = {0};
    if (stat(sciezka, &st) == -1) {
        mkdir(sciezka, 0777);
    }
}

// Pobiera aktualny czas z milisekundami do logów
static inline void pobierz_czas(char *buffer, size_t size) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *tm_info = localtime(&tv.tv_sec);
    int millis = tv.tv_usec / 1000;
    snprintf(buffer, size, "%02d:%02d:%02d.%03d", 
             tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, millis);
}

// Otwiera plik logu z datą i godziną w nazwie
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

// Funkcja logująca - wypisuje kolorowy tekst na ekran i czysty tekst do pliku
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

// --- STRUKTURY DANYCH ---

// Krótkie zgłoszenie wysyłane przez kandydata do FIFO (tylko najpilniejsze dane)
typedef struct {
    pid_t id;
    int zdana_matura;        
    int zdana_teoria_wczesniej; 
} Zgloszenie;

// Pełna teczka studenta w pamięci dzielonej (dostępna dla komisji)
typedef struct {
    pid_t id;
    int zdana_matura;
    int zdana_teoria_wczesniej;
    double punkty_teoria;   
    double punkty_praktyka; 
    int status;           // Globalny status (np. 1-czeka na A, 2-czeka na B, 3-koniec)
    int czy_przyjety;    
    
    // Arkusz egzaminacyjny (max 5 pytań)
    int pytania[5];             
    int id_egzaminatora[5];     
    int odpowiedzi[5];          
    int oceny[5];               
    int status_arkusza; // Status komunikacji z komisją (1-masz pytania, 2-mam odpowiedzi)
    
    int zaliczona_A; 
    int zaliczona_B; 
    
    // Mechanizmy synchronizacji indywidulane dla każdego studenta
    pthread_mutex_t mutex_ipc;
    pthread_cond_t  cond_ipc;
} Student;

// Główna struktura pamięci dzielonej - zawiera listę wszystkich studentów
typedef struct {
    Student lista[MAX_KANDYDATOW];
    int liczba_kandydatow;
    
    // Synchronizacja dostępu do listy (żeby dwóch nie zapisało się w tym samym miejscu)
    pthread_mutex_t mutex_rejestracji;
    pthread_cond_t  cond_rejestracji;
} EgzaminPamiecDzielona;

// Konfiguracja sali egzaminacyjnej
#define MAX_MIEJSC 3             // Liczba stolików w sali
#define LICZBA_EGZAMINATOROW_A 5 // Skład komisji A (1 Przew. + 4 Członków)
#define LICZBA_EGZAMINATOROW_B 3 // Skład komisji B (1 Przew. + 2 Członków)
#define LICZBA_KOMISJI 2

// Struktura opisująca pojedynczy stolik egzaminacyjny (lokalna dla procesu komisji)
typedef struct {
    int id_kandydata;              // Kto siedzi przy stoliku?
    int pytania[5];                // Historia pytań przy tym stoliku (żeby nie powtarzać)
    int liczba_zadanych_pytan;     
    int zajete;                    // Czy stolik jest wolny?
    int potrzebni_czlonkowie;      // Ilu egzaminatorów jeszcze brakuje?
    
    pthread_mutex_t mutex;         // Ochrona danych stolika
    pthread_barrier_t bariera;     // Synchronizacja startu/końca egzaminu przy stoliku
} Stanowisko;

#endif