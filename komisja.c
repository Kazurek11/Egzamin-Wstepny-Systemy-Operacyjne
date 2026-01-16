#define _GNU_SOURCE // Konieczne, aby używać syscall()
#include "common.h"
#include <sys/syscall.h> // Konieczne dla stałej SYS_gettid
#include <unistd.h>      // Konieczne dla funkcji syscall()

// --- ZMIENNE GLOBALNE DLA PROCESU KOMISJI ---
Stanowisko stanowiska[MAX_MIEJSC];
FILE *plik_logu;
EgzaminPamiecDzielona *egzamin;

// Semafory
sem_t *sem_kolejka_komisji;
sem_t *sem_miejsca_komisji;
sem_t *sem_kolejka_przyszlosci; // Tylko dla A (wskazuje na kolejkę B)
sem_t *sem_licznik_konca;

// Mutexy i Cond wewnątrz procesu
pthread_mutex_t mutex_rekrutacja = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_wolni_czlonkowie = PTHREAD_COND_INITIALIZER;

// --- FUNKCJE POMOCNICZE ---

// Funkcja czyszcząca pamięć kandydata przed wysłaniem go do następnej komisji
void cleaner_pytan(int id_kandydata) {
    if (id_kandydata < 0 || id_kandydata >= MAX_KANDYDATOW) return;
    for (int i = 0; i < 5; i++) {
        egzamin->lista[id_kandydata].pytania[i] = -1;
        egzamin->lista[id_kandydata].id_egzaminatora[i] = -1;
        egzamin->lista[id_kandydata].odpowiedzi[i] = -1;
        egzamin->lista[id_kandydata].oceny[i] = -1; 
    }
}

int znajdz_wolne_stanowisko() {
    for (int i = 0; i < MAX_MIEJSC; i++) {
        pthread_mutex_lock(&stanowiska[i].mutex);
        if (stanowiska[i].zajete == 0) {
            stanowiska[i].zajete = 1; // Rezerwacja wstępna
            pthread_mutex_unlock(&stanowiska[i].mutex);
            return i;
        }
        pthread_mutex_unlock(&stanowiska[i].mutex);
    }
    return -1; 
}

int wylosuj_unikalne_pytanie(int stolik_idx) {
    int unikalne = 0;
    int pytanie = 0;
    while (!unikalne) {
        pytanie = (rand() % 50) + 1; 
        unikalne = 1;
        for (int i = 0; i < stanowiska[stolik_idx].liczba_zadanych_pytan; i++) {
            if (stanowiska[stolik_idx].pytania[i] == pytanie) {
                unikalne = 0;
                break;
            }
        }
    }
    return pytanie;
}

// --- UNIWERSALNA FUNKCJA EGZAMINOWANIA (ZADAWANIE PYTAŃ) ---
void pracuj_jako_czlonek(int stolik, int id, char typ_egzaminatora, char typ_komisji) {
    // 1. Bariera startowa - czekamy na komplet przy stoliku
    pthread_barrier_wait(&stanowiska[stolik].bariera);

    // 2. Zadawanie pytania (Sekcja Krytyczna Stanowiska)
    pthread_mutex_lock(&stanowiska[stolik].mutex);
    
    int pyt = wylosuj_unikalne_pytanie(stolik);
    int idx = stanowiska[stolik].liczba_zadanych_pytan;
    stanowiska[stolik].pytania[idx] = pyt;
    stanowiska[stolik].liczba_zadanych_pytan++;
    
    int pid_kandydata = stanowiska[stolik].id_kandydata;
    
    // Szukamy indeksu w SHM
    int shm_idx = -1;
    for (int k = 0; k < egzamin->liczba_kandydatow; k++) {
        if (egzamin->lista[k].id == pid_kandydata) {
            shm_idx = k;
            break;
        }
    }
    
    if (shm_idx != -1) {
        egzamin->lista[shm_idx].pytania[idx] = pyt;
        egzamin->lista[shm_idx].id_egzaminatora[idx] = id;
        
        // Logowanie faktu zadania pytania
        dodaj_do_loggera(plik_logu, "[Komisja %c] [PID: %d] |\t Pytanie nr: %d |\t Egzaminator [%c] [TID: %ld] [ID: %d] |\t Zadał kandydatowi [PID: %d] treść nr: %d\n",
            typ_komisji, (int)getpid(), idx + 1, typ_egzaminatora, (long)syscall(SYS_gettid), id, pid_kandydata, pyt);
    }

    pthread_mutex_unlock(&stanowiska[stolik].mutex);

    // 3. Bariera po zadaniu pytań
    pthread_barrier_wait(&stanowiska[stolik].bariera);
}

// ================= KOMISJA A =================

void* czlonek_komisji_A(void* arg){
    int id = *(int*)arg;
    free(arg);
    while (1) {
        int moj_stolik = -1;
        
        // Szukanie pracy
        pthread_mutex_lock(&mutex_rekrutacja);
        while (moj_stolik == -1) {
            for (int i = 0; i < MAX_MIEJSC; i++) {
                if (stanowiska[i].potrzebni_czlonkowie > 0) {
                    stanowiska[i].potrzebni_czlonkowie--;
                    moj_stolik = i;
                    break;
                }
            }
            if (moj_stolik == -1) {
                pthread_cond_wait(&cond_wolni_czlonkowie, &mutex_rekrutacja);
            }
        }
        pthread_mutex_unlock(&mutex_rekrutacja);

        // Praca - zadawanie pytań
        pracuj_jako_czlonek(moj_stolik, id, 'C', 'A');

        // Czekanie na barierze aż student odpowie
        pthread_barrier_wait(&stanowiska[moj_stolik].bariera);

        // --- OCENIANIE ---
        int pid_kandydata = stanowiska[moj_stolik].id_kandydata;
        int shm_idx = -1;
        for (int k = 0; k < egzamin->liczba_kandydatow; k++) {
             if (egzamin->lista[k].id == pid_kandydata) { shm_idx = k; break; }
        }

        if (shm_idx != -1) {
            Student *s = &egzamin->lista[shm_idx];
            for (int k = 0; k < 5; k++) {
                if (s->id_egzaminatora[k] == id && s->oceny[k] == -1) {
                    int ocena = 0;
                    if (s->status_arkusza == 2) ocena = (rand() % 101); 
                    s->oceny[k] = ocena;

                    // Logowanie oceny
                    dodaj_do_loggera(plik_logu, "[Komisja A] [PID: %d] |\t Egzaminator [C] [TID: %ld] [ID: %d] |\t Ocenił kandydata [PID: %d] | Numer pytania: %d | Jego odpowiedz: %d \t Wynik cząstkowy: %d\n",
                        (int)getpid(), (long)syscall(SYS_gettid), id, s->id, s->pytania[k], s->odpowiedzi[k], ocena);
                }
            }
        }

        // Bariera końcowa
        pthread_barrier_wait(&stanowiska[moj_stolik].bariera);
    }
    return NULL;
}

void* przewodniczacy_komisji_A(void* arg){
    int P_id = *(int*)arg;
    free(arg); 
    dodaj_do_loggera(plik_logu, "[Komisja A] [P] Przewodniczący %d w komisji A: Rozpoczynam pracę.\n", P_id);

    while (1) {
        sem_wait(sem_kolejka_komisji); 
        sem_wait(sem_miejsca_komisji); 

        int stolik = znajdz_wolne_stanowisko();
        
        pthread_mutex_lock(&stanowiska[stolik].mutex);
        int id_kandydata_PD = -1;
        int kandydat_pid = -1;

        for (int i = 0; i < egzamin->liczba_kandydatow; i++) {
            if (egzamin->lista[i].status == 1) { 
                egzamin->lista[i].status = 11;  
                id_kandydata_PD = i;
                kandydat_pid = egzamin->lista[i].id;
                break;
            }
        }
        
        if (id_kandydata_PD == -1) {
            stanowiska[stolik].zajete = 0;
            pthread_mutex_unlock(&stanowiska[stolik].mutex);
            sem_post(sem_miejsca_komisji);
            sem_post(sem_kolejka_komisji);
            continue; 
        }

        // Obsługa "Starej Teorii"
        if (egzamin->lista[id_kandydata_PD].zdana_teoria_wczesniej == 1) {
            dodaj_do_loggera(plik_logu, "[Komisja A] [PID: %d] |\t Kandydat [PID: %d] (Stary) |\t ZALICZONA A (Wcześniej) |\t Przekierowanie do B\n", 
                   (int)getpid(), kandydat_pid);
            
            pthread_mutex_lock(&egzamin->lista[id_kandydata_PD].mutex_ipc);
            egzamin->lista[id_kandydata_PD].zaliczona_A = 1;
            cleaner_pytan(id_kandydata_PD); 
            egzamin->lista[id_kandydata_PD].status = 2; 
            pthread_mutex_unlock(&egzamin->lista[id_kandydata_PD].mutex_ipc);

            sem_post(sem_kolejka_przyszlosci); 
            
            stanowiska[stolik].zajete = 0;
            pthread_mutex_unlock(&stanowiska[stolik].mutex);
            sem_post(sem_miejsca_komisji);
            continue; 
        }

        stanowiska[stolik].id_kandydata = kandydat_pid;
        stanowiska[stolik].liczba_zadanych_pytan = 0;
        
        for (int i = 0; i < 5; i++) {
            egzamin->lista[id_kandydata_PD].oceny[i] = -1;
            egzamin->lista[id_kandydata_PD].id_egzaminatora[i] = -1;
        }
        pthread_mutex_unlock(&stanowiska[stolik].mutex);

        // Rekrutacja
        pthread_mutex_lock(&mutex_rekrutacja);
        stanowiska[stolik].potrzebni_czlonkowie = LICZBA_EGZAMINATOROW_A - 1;
        pthread_cond_broadcast(&cond_wolni_czlonkowie);
        pthread_mutex_unlock(&mutex_rekrutacja);

        // Bariera 1: Start
        pthread_barrier_wait(&stanowiska[stolik].bariera);

        // Przewodniczący też zadaje pytanie
        pthread_mutex_lock(&stanowiska[stolik].mutex);
        int pyt = wylosuj_unikalne_pytanie(stolik);
        int idx = stanowiska[stolik].liczba_zadanych_pytan;
        stanowiska[stolik].pytania[idx] = pyt;
        stanowiska[stolik].liczba_zadanych_pytan++;
        
        egzamin->lista[id_kandydata_PD].pytania[idx] = pyt;
        egzamin->lista[id_kandydata_PD].id_egzaminatora[idx] = P_id;
        
        dodaj_do_loggera(plik_logu, "[Komisja A] [PID: %d] |\t Pytanie nr: %d |\t Egzaminator [P] [TID: %ld] [ID: %d] |\t Zadał kandydatowi [PID: %d] treść nr: %d\n",
            (int)getpid(), idx + 1, (long)syscall(SYS_gettid), P_id, kandydat_pid, pyt);

        pthread_mutex_unlock(&stanowiska[stolik].mutex);

        // Bariera 2: Pytania zadane
        pthread_barrier_wait(&stanowiska[stolik].bariera);
        
        // Komunikacja
        pthread_mutex_lock(&egzamin->lista[id_kandydata_PD].mutex_ipc);
        egzamin->lista[id_kandydata_PD].status_arkusza = 1; 
        pthread_cond_signal(&egzamin->lista[id_kandydata_PD].cond_ipc); 
        
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 2; 

        int wait_res = 0;
        while (egzamin->lista[id_kandydata_PD].status_arkusza != 2 && wait_res != ETIMEDOUT) {
            wait_res = pthread_cond_timedwait(&egzamin->lista[id_kandydata_PD].cond_ipc, 
                                              &egzamin->lista[id_kandydata_PD].mutex_ipc, 
                                              &ts);
        }
        
        if (wait_res == ETIMEDOUT) {
             dodaj_do_loggera(plik_logu, "[Komisja A] [PID: %d] TIMEOUT! Kandydat [PID: %d] nie odpowiedział. Dyskwalifikacja.\n", (int)getpid(), kandydat_pid);
             egzamin->lista[id_kandydata_PD].status = 3; 
             pthread_cond_signal(&egzamin->lista[id_kandydata_PD].cond_ipc); 
        }
        pthread_mutex_unlock(&egzamin->lista[id_kandydata_PD].mutex_ipc);

        // Bariera 3: Pozwolenie na ocenianie
        pthread_barrier_wait(&stanowiska[stolik].bariera); 

        // Ocenianie przez P
        Student *s = &egzamin->lista[id_kandydata_PD];
        for (int k = 0; k < 5; k++) {
            if (s->id_egzaminatora[k] == P_id && s->oceny[k] == -1) {
                int ocena = 0;
                if (s->status_arkusza == 2) ocena = (rand() % 101); 
                s->oceny[k] = ocena;
                
                dodaj_do_loggera(plik_logu, "[Komisja A] [PID: %d] |\t Egzaminator [P] [TID: %ld] [ID: %d] |\t Ocenił kandydata [PID: %d] | Numer pytania: %d | Jego odpowiedz: %d \t Wynik cząstkowy: %d\n",
                        (int)getpid(), (long)syscall(SYS_gettid), P_id, s->id, s->pytania[k], s->odpowiedzi[k], ocena);
            }
        }

        // Bariera 4: Koniec oceniania
        pthread_barrier_wait(&stanowiska[stolik].bariera);

        // Podsumowanie
        int suma = 0;
        for (int k = 0; k < LICZBA_EGZAMINATOROW_A; k++) {
            suma += egzamin->lista[id_kandydata_PD].oceny[k];
        }
        double srednia = round(((double)suma / LICZBA_EGZAMINATOROW_A) * 100.0) / 100.0;
        egzamin->lista[id_kandydata_PD].punkty_teoria = srednia;
        
        pthread_mutex_lock(&egzamin->lista[id_kandydata_PD].mutex_ipc);
        
        if (srednia >= 30.0 && s->status != 3) {
            egzamin->lista[id_kandydata_PD].zaliczona_A = 1; 
            dodaj_do_loggera(plik_logu, "[Komisja A] [PID: %d] |\t Kandydat [PID: %d] |\t ZDAŁ A (-> B) |\t Wynik: %.2lf pkt\n", 
                   (int)getpid(), kandydat_pid, srednia);
            
            cleaner_pytan(id_kandydata_PD);

            egzamin->lista[id_kandydata_PD].status = 2; 
            sem_post(sem_kolejka_przyszlosci);
        } else {
            egzamin->lista[id_kandydata_PD].zaliczona_A = 0; 
            dodaj_do_loggera(plik_logu, "[Komisja A] [PID: %d] |\t Kandydat [PID: %d] |\t OBLAŁ A (Koniec) |\t Wynik: %.2lf pkt\n", 
                   (int)getpid(), kandydat_pid, srednia);
                   
            egzamin->lista[id_kandydata_PD].status = 3; 
            sem_post(sem_licznik_konca);
        }

        egzamin->lista[id_kandydata_PD].status_arkusza = 0; 
        
        pthread_cond_signal(&egzamin->lista[id_kandydata_PD].cond_ipc); 
        pthread_mutex_unlock(&egzamin->lista[id_kandydata_PD].mutex_ipc);
        
        stanowiska[stolik].zajete = 0; 
        sem_post(sem_miejsca_komisji); 
    }
    return NULL;
}

// ================= KOMISJA B =================

void* czlonek_komisji_B(void* arg){
    int id = *(int*)arg;
    free(arg);
    while (1) {
        int moj_stolik = -1;
        pthread_mutex_lock(&mutex_rekrutacja);
        while (moj_stolik == -1) {
            for (int i = 0; i < MAX_MIEJSC; i++) {
                if (stanowiska[i].potrzebni_czlonkowie > 0) {
                    stanowiska[i].potrzebni_czlonkowie--;
                    moj_stolik = i;
                    break;
                }
            }
            if (moj_stolik == -1) {
                pthread_cond_wait(&cond_wolni_czlonkowie, &mutex_rekrutacja);
            }
        }
        pthread_mutex_unlock(&mutex_rekrutacja);

        pracuj_jako_czlonek(moj_stolik, id, 'C', 'B');

        pthread_barrier_wait(&stanowiska[moj_stolik].bariera);

        // Ocenianie B
        int pid_kandydata = stanowiska[moj_stolik].id_kandydata;
        int shm_idx = -1;
        for (int k = 0; k < egzamin->liczba_kandydatow; k++) {
             if (egzamin->lista[k].id == pid_kandydata) { shm_idx = k; break; }
        }

        if (shm_idx != -1) {
            Student *s = &egzamin->lista[shm_idx];
            for (int k = 0; k < 5; k++) {
                if (s->id_egzaminatora[k] == id && s->oceny[k] == -1) {
                    int ocena = 0;
                    if (s->status_arkusza == 2) ocena = (rand() % 101); 
                    s->oceny[k] = ocena;

                    dodaj_do_loggera(plik_logu, "[Komisja B] [PID: %d] |\t Egzaminator [C] [TID: %ld] [ID: %d] |\t Ocenił kandydata [PID: %d] | Numer pytania: %d | Jego odpowiedz: %d \t Wynik cząstkowy: %d\n",
                        (int)getpid(), (long)syscall(SYS_gettid), id, s->id, s->pytania[k], s->odpowiedzi[k], ocena);
                }
            }
        }

        pthread_barrier_wait(&stanowiska[moj_stolik].bariera);
    }
    return NULL;
}

void* przewodniczacy_komisji_B(void* arg){
    int P_id = *(int*)arg;
    free(arg); 
    dodaj_do_loggera(plik_logu, "[Komisja B] [P] Przewodniczący %d w komisji B: Rozpoczynam pracę.\n", P_id);

    while (1) {
        sem_wait(sem_kolejka_komisji); 
        sem_wait(sem_miejsca_komisji); 

        int stolik = znajdz_wolne_stanowisko();
        
        pthread_mutex_lock(&stanowiska[stolik].mutex);
        int id_kandydata_PD = -1;
        int kandydat_pid = -1;

        for (int i = 0; i < egzamin->liczba_kandydatow; i++) {
            if (egzamin->lista[i].status == 2) { 
                egzamin->lista[i].status = 22; 
                id_kandydata_PD = i;
                kandydat_pid = egzamin->lista[i].id;
                break;
            }
        }
        
        if (id_kandydata_PD == -1) {
            stanowiska[stolik].zajete = 0;
            pthread_mutex_unlock(&stanowiska[stolik].mutex);
            sem_post(sem_miejsca_komisji);
            sem_post(sem_kolejka_komisji);
            continue; 
        }

        stanowiska[stolik].id_kandydata = kandydat_pid;
        stanowiska[stolik].liczba_zadanych_pytan = 0;
        
        // Reset lokalny dla B
        for (int i = 0; i < 5; i++) {
            egzamin->lista[id_kandydata_PD].oceny[i] = -1;
            egzamin->lista[id_kandydata_PD].id_egzaminatora[i] = -1;
        }

        pthread_mutex_unlock(&stanowiska[stolik].mutex);

        pthread_mutex_lock(&mutex_rekrutacja);
        stanowiska[stolik].potrzebni_czlonkowie = LICZBA_EGZAMINATOROW_B - 1;
        pthread_cond_broadcast(&cond_wolni_czlonkowie);
        pthread_mutex_unlock(&mutex_rekrutacja);

        pthread_barrier_wait(&stanowiska[stolik].bariera);

        // P też zadaje pytanie
        pthread_mutex_lock(&stanowiska[stolik].mutex);
        int pyt = wylosuj_unikalne_pytanie(stolik);
        int idx = stanowiska[stolik].liczba_zadanych_pytan;
        stanowiska[stolik].pytania[idx] = pyt;
        stanowiska[stolik].liczba_zadanych_pytan++;
        
        egzamin->lista[id_kandydata_PD].pytania[idx] = pyt;
        egzamin->lista[id_kandydata_PD].id_egzaminatora[idx] = P_id;
        
        dodaj_do_loggera(plik_logu, "[Komisja B] [PID: %d] |\t Pytanie nr: %d |\t Egzaminator [P] [TID: %ld] [ID: %d] |\t Zadał kandydatowi [PID: %d] treść nr: %d\n",
            (int)getpid(), idx + 1, (long)syscall(SYS_gettid), P_id, kandydat_pid, pyt);

        pthread_mutex_unlock(&stanowiska[stolik].mutex);

        pthread_barrier_wait(&stanowiska[stolik].bariera);
        
        // Timed Wait
        pthread_mutex_lock(&egzamin->lista[id_kandydata_PD].mutex_ipc);
        egzamin->lista[id_kandydata_PD].status_arkusza = 1; 
        pthread_cond_signal(&egzamin->lista[id_kandydata_PD].cond_ipc); 
        
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 2; 

        int wait_res = 0;
        while (egzamin->lista[id_kandydata_PD].status_arkusza != 2 && wait_res != ETIMEDOUT) {
            wait_res = pthread_cond_timedwait(&egzamin->lista[id_kandydata_PD].cond_ipc, 
                                              &egzamin->lista[id_kandydata_PD].mutex_ipc, 
                                              &ts);
        }
        
        if (wait_res == ETIMEDOUT) {
             dodaj_do_loggera(plik_logu, "[Komisja B] [PID: %d] TIMEOUT! Dyskwalifikacja.\n", (int)getpid());
             egzamin->lista[id_kandydata_PD].status = 3; 
             pthread_cond_signal(&egzamin->lista[id_kandydata_PD].cond_ipc); 
        }
        pthread_mutex_unlock(&egzamin->lista[id_kandydata_PD].mutex_ipc);

        pthread_barrier_wait(&stanowiska[stolik].bariera); 

        // Ocenianie przez P
        Student *s = &egzamin->lista[id_kandydata_PD];
        for (int k = 0; k < 5; k++) {
            if (s->id_egzaminatora[k] == P_id && s->oceny[k] == -1) {
                int ocena = 0;
                if (s->status_arkusza == 2) ocena = (rand() % 101); 
                s->oceny[k] = ocena;
                
                dodaj_do_loggera(plik_logu, "[Komisja B] [PID: %d] |\t Egzaminator [P] [TID: %ld] [ID: %d] |\t Ocenił kandydata [PID: %d] | Numer pytania: %d | Jego odpowiedz: %d \t Wynik cząstkowy: %d\n",
                        (int)getpid(), (long)syscall(SYS_gettid), P_id, s->id, s->pytania[k], s->odpowiedzi[k], ocena);
            }
        }

        pthread_barrier_wait(&stanowiska[stolik].bariera);

        int suma = 0;
        for (int k = 0; k < LICZBA_EGZAMINATOROW_B; k++) {
            suma += egzamin->lista[id_kandydata_PD].oceny[k];
        }
        double srednia = round(((double)suma / LICZBA_EGZAMINATOROW_B) * 100.0) / 100.0;
        egzamin->lista[id_kandydata_PD].punkty_praktyka = srednia;
        
        pthread_mutex_lock(&egzamin->lista[id_kandydata_PD].mutex_ipc);
        
        if (srednia >= 30.0 && s->status != 3) {
            egzamin->lista[id_kandydata_PD].zaliczona_B = 1;
            egzamin->lista[id_kandydata_PD].czy_przyjety = 1; 
            dodaj_do_loggera(plik_logu, "[Komisja B] [PID: %d] |\t Kandydat [PID: %d] |\t ZDAŁ B (Przyjęty) |\t Wynik: %.2lf pkt\n", 
                   (int)getpid(), kandydat_pid, srednia);
            
            egzamin->lista[id_kandydata_PD].status = 3; 
        } else {
            egzamin->lista[id_kandydata_PD].zaliczona_B = 0; 
            dodaj_do_loggera(plik_logu, "[Komisja B] [PID: %d] |\t Kandydat [PID: %d] |\t OBLAŁ B (Koniec) |\t Wynik: %.2lf pkt\n", 
                   (int)getpid(), kandydat_pid, srednia);
                   
            egzamin->lista[id_kandydata_PD].status = 3; 
        }

        egzamin->lista[id_kandydata_PD].status_arkusza = 0; 
        sem_post(sem_licznik_konca); // Koniec dla Dziekana

        pthread_cond_signal(&egzamin->lista[id_kandydata_PD].cond_ipc); 
        pthread_mutex_unlock(&egzamin->lista[id_kandydata_PD].mutex_ipc);
        
        stanowiska[stolik].zajete = 0; 
        sem_post(sem_miejsca_komisji); 
    }
    return NULL;
}

// --- MAIN KOMISJI ---
int main(int argc, char *argv[]) {
    if (argc < 2) return 1;
    char typ_komisji = argv[1][0];
    
    char nazwa_loga[32];
    sprintf(nazwa_loga, "komisja_%c.log", typ_komisji);
    plik_logu = otworz_log(nazwa_loga, "w");
    AKTUALNY_KOLOR_LOGU = (typ_komisji == 'A') ? ANSI_COLOR_YELLOW : ANSI_COLOR_CYAN;

    srand(getpid());

    // Inicjalizacja stanowisk
    for (int i = 0; i < MAX_MIEJSC; i++) {
        stanowiska[i].zajete = 0;
        stanowiska[i].potrzebni_czlonkowie = 0;
        pthread_mutex_init(&stanowiska[i].mutex, NULL);
        
        int n = (typ_komisji == 'A') ? LICZBA_EGZAMINATOROW_A : LICZBA_EGZAMINATOROW_B;
        pthread_barrier_init(&stanowiska[i].bariera, NULL, n);
    }

    // Pamięć dzielona
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0600);
    if (shm_fd == -1) {
        if (plik_logu) fclose(plik_logu);
        return 4;
    }
    egzamin = mmap(NULL, sizeof(EgzaminPamiecDzielona), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    
    sem_licznik_konca = sem_open(SEM_LICZNIK_KONCA, 0);

    if (typ_komisji == 'A'){
        sem_kolejka_komisji = sem_open(KOLEJKA_KOMISJA_A, 0);
        sem_miejsca_komisji = sem_open(WOLNE_MIEJSCA_KOMISJA_A, 0);
        sem_kolejka_przyszlosci = sem_open(KOLEJKA_KOMISJA_B, 0);
    } else {
        sem_kolejka_komisji = sem_open(KOLEJKA_KOMISJA_B, 0);
        sem_miejsca_komisji = sem_open(WOLNE_MIEJSCA_KOMISJA_B, 0);
        sem_kolejka_przyszlosci = NULL; 
    }

    if (sem_kolejka_komisji == SEM_FAILED || sem_miejsca_komisji == SEM_FAILED || sem_licznik_konca == SEM_FAILED) {
        if (plik_logu) fclose(plik_logu);
        return 6;
    }

    // Tworzenie wątków
    int liczba_czlonkow = (typ_komisji == 'A') ? LICZBA_EGZAMINATOROW_A : LICZBA_EGZAMINATOROW_B;
    pthread_t *egzaminatorzy = malloc(sizeof(pthread_t) * liczba_czlonkow);

    for (int i = 0; i < liczba_czlonkow; i++) {
        int *numer_egzaminujacego = malloc(sizeof(int));
        *numer_egzaminujacego = i;
        
        if (typ_komisji == 'A') {
            if (i == 0) pthread_create(&egzaminatorzy[i], NULL, &przewodniczacy_komisji_A, numer_egzaminujacego);
            else pthread_create(&egzaminatorzy[i], NULL, &czlonek_komisji_A, numer_egzaminujacego);
        } else {
            if (i == 0) pthread_create(&egzaminatorzy[i], NULL, &przewodniczacy_komisji_B, numer_egzaminujacego);
            else pthread_create(&egzaminatorzy[i], NULL, &czlonek_komisji_B, numer_egzaminujacego);
        }
    }

    // Czekanie na wątki (nieskończoność w teorii, ale zabije nas Dziekan sygnałem)
    for (int i = 0; i < liczba_czlonkow; i++) {
        pthread_join(egzaminatorzy[i], NULL);
    }

    free(egzaminatorzy);
    if (plik_logu) fclose(plik_logu);
    return 0;
}