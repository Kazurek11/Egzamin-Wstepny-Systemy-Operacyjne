#define _GNU_SOURCE
#include "common.h"
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <math.h>     
#include <stdlib.h>   
#include <time.h>     

FILE *plik_logu;

EgzaminPamiecDzielona *egzamin;
Stanowisko stanowiska[MAX_MIEJSC]; 

pthread_mutex_t mutex_szukania = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mutex_rekrutacja = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_wolni_czlonkowie = PTHREAD_COND_INITIALIZER;

sem_t *sem_kolejka_komisji;
sem_t *sem_miejsca_komisji;
sem_t *sem_kolejka_przyszlosci; 

int znajdz_wolne_stanowisko() {
    int znaleziony = -1;
    pthread_mutex_lock(&mutex_szukania);
    for (int i = 0; i < MAX_MIEJSC; i++) {
        if (stanowiska[i].zajete == 0) {
            stanowiska[i].zajete = 1; 
            znaleziony = i;
            break;
        }
    }
    pthread_mutex_unlock(&mutex_szukania);
    return znaleziony;
}

int wylosuj_unikalne_pytanie(int stolik) {
    int unikalne = 0;
    int pytanie;
    while (!unikalne) {
        pytanie = (rand() % 50) + 1;
        unikalne = 1;
        for (int i = 0; i < stanowiska[stolik].liczba_zadanych_pytan; i++) {
            if (stanowiska[stolik].pytania[i] == pytanie) {
                unikalne = 0; 
                break;
            }
        }
    }
    return pytanie;
}

void pracuj_jako_czlonek(int stolik, int id, char rola, char komisja) {
    pthread_barrier_wait(&stanowiska[stolik].bariera);

    pthread_mutex_lock(&stanowiska[stolik].mutex);
    
    int pyt = wylosuj_unikalne_pytanie(stolik);
    int idx = stanowiska[stolik].liczba_zadanych_pytan;
    stanowiska[stolik].pytania[idx] = pyt;
    stanowiska[stolik].liczba_zadanych_pytan++;
    
    int pid_kandydata = stanowiska[stolik].id_kandydata;
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
    }
    
    dodaj_do_loggera(plik_logu, "[Komisja %c] [PID: %d] |\t Pytanie nr: %d |\t Egzaminator [%c] [TID: %ld] [ID: %d] |\t Zadał kandydatowi [PID: %d] treść nr: %d\n",
       komisja,
       (int)getpid(),
       idx + 1,
       rola,
       (long)syscall(SYS_gettid),
       id,
       pid_kandydata,
       pyt);
    
    pthread_mutex_unlock(&stanowiska[stolik].mutex);

    pthread_barrier_wait(&stanowiska[stolik].bariera);

    pthread_barrier_wait(&stanowiska[stolik].bariera);
    
    if (shm_idx != -1) {
        Student *s = &egzamin->lista[shm_idx];
        for (int k = 0; k < 5; k++) {
            if (s->id_egzaminatora[k] == id && s->oceny[k] == -1) {
                int ocena = (rand() % 101);
                s->oceny[k] = ocena;
                
                dodaj_do_loggera(plik_logu, "[Komisja %c] [PID: %d] |\t Egzaminator [%c] [TID: %ld] [ID: %d] |\t Ocenił kandydata [PID: %d] |\t Wynik cząstkowy: %d\n",
                       komisja,
                       (int)getpid(),
                       rola,
                       (long)syscall(SYS_gettid),
                       id,
                       s->id,
                       ocena);
            }
        }
    }

    pthread_barrier_wait(&stanowiska[stolik].bariera);
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
            continue; 
        }

        if (egzamin->lista[id_kandydata_PD].zdana_teoria_wczesniej == 1) {
            dodaj_do_loggera(plik_logu, "[Komisja A] [PID: %d] |\t Kandydat [PID: %d] (Stary) |\t ZALICZONA A (Wcześniej) |\t Przekierowanie do B\n", 
                   (int)getpid(), kandydat_pid);
            
            egzamin->lista[id_kandydata_PD].zaliczona_A = 1;
            egzamin->lista[id_kandydata_PD].status = 2; 
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

        pthread_mutex_lock(&mutex_rekrutacja);
        stanowiska[stolik].potrzebni_czlonkowie = LICZBA_EGZAMINATOROW_A - 1;
        pthread_cond_broadcast(&cond_wolni_czlonkowie);
        pthread_mutex_unlock(&mutex_rekrutacja);

        pthread_barrier_wait(&stanowiska[stolik].bariera);

        pthread_mutex_lock(&stanowiska[stolik].mutex);
        int pyt = wylosuj_unikalne_pytanie(stolik);
        int idx = stanowiska[stolik].liczba_zadanych_pytan;
        stanowiska[stolik].pytania[idx] = pyt;
        stanowiska[stolik].liczba_zadanych_pytan++;
        
        egzamin->lista[id_kandydata_PD].pytania[idx] = pyt;
        egzamin->lista[id_kandydata_PD].id_egzaminatora[idx] = P_id;
        
        dodaj_do_loggera(plik_logu, "[Komisja A] [PID: %d] |\t Pytanie nr: %d |\t Egzaminator [P] [TID: %ld] [ID: %d] |\t Zadał kandydatowi [PID: %d] treść nr: %d\n",
            (int)getpid(),
            idx + 1,
            (long)syscall(SYS_gettid),
            P_id,
            kandydat_pid,
            pyt);

        pthread_mutex_unlock(&stanowiska[stolik].mutex);

        pthread_barrier_wait(&stanowiska[stolik].bariera);
        
        pthread_mutex_lock(&egzamin->lista[id_kandydata_PD].mutex_ipc);
        egzamin->lista[id_kandydata_PD].status_arkusza = 1; 
        pthread_cond_signal(&egzamin->lista[id_kandydata_PD].cond_ipc); 
        
        while (egzamin->lista[id_kandydata_PD].status_arkusza != 2) {
            pthread_cond_wait(&egzamin->lista[id_kandydata_PD].cond_ipc, &egzamin->lista[id_kandydata_PD].mutex_ipc);
        }
        pthread_mutex_unlock(&egzamin->lista[id_kandydata_PD].mutex_ipc);

        pthread_barrier_wait(&stanowiska[stolik].bariera);

        Student *s = &egzamin->lista[id_kandydata_PD];
        for (int k = 0; k < 5; k++) {
            if (s->id_egzaminatora[k] == P_id && s->oceny[k] == -1) {
                int ocena = (rand() % 101);
                s->oceny[k] = ocena;
                
                dodaj_do_loggera(plik_logu, "[Komisja A] [PID: %d] |\t Egzaminator [P] [TID: %ld] [ID: %d] |\t Ocenił kandydata [PID: %d] |\t Wynik cząstkowy: %d\n",
                       (int)getpid(),
                       (long)syscall(SYS_gettid),
                       P_id,
                       s->id,
                       ocena);
            }
        }

        pthread_barrier_wait(&stanowiska[stolik].bariera);

        int suma = 0;
        for (int k = 0; k < LICZBA_EGZAMINATOROW_A; k++) {
            suma += egzamin->lista[id_kandydata_PD].oceny[k];
        }
        double srednia = round(((double)suma / LICZBA_EGZAMINATOROW_A) * 100.0) / 100.0;
        egzamin->lista[id_kandydata_PD].punkty_teoria = srednia;
        
        pthread_mutex_lock(&egzamin->lista[id_kandydata_PD].mutex_ipc);
        if (srednia >= 30.0) {
            egzamin->lista[id_kandydata_PD].zaliczona_A = 1; 
            dodaj_do_loggera(plik_logu, "[Komisja A] [PID: %d] |\t Kandydat [PID: %d] |\t ZDAŁ A (-> B) |\t Wynik: %.2lf pkt\n", 
                   (int)getpid(), kandydat_pid, srednia);
                   
            egzamin->lista[id_kandydata_PD].status = 2; 
            sem_post(sem_kolejka_przyszlosci);
        } else {
            egzamin->lista[id_kandydata_PD].zaliczona_A = 0; 
            dodaj_do_loggera(plik_logu, "[Komisja A] [PID: %d] |\t Kandydat [PID: %d] |\t OBLAŁ A (Koniec) |\t Wynik: %.2lf pkt\n", 
                   (int)getpid(), kandydat_pid, srednia);
                   
            egzamin->lista[id_kandydata_PD].status = 3; 
        }
        pthread_cond_signal(&egzamin->lista[id_kandydata_PD].cond_ipc); 
        pthread_mutex_unlock(&egzamin->lista[id_kandydata_PD].mutex_ipc);
        
        stanowiska[stolik].zajete = 0; 
        sem_post(sem_miejsca_komisji); 
    }
    return NULL;
}

void* czlonek_komisji_A(void* arg){
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

        pracuj_jako_czlonek(moj_stolik, id, 'C', 'A');
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

        pthread_mutex_lock(&stanowiska[stolik].mutex);
        int pyt = wylosuj_unikalne_pytanie(stolik);
        int idx = stanowiska[stolik].liczba_zadanych_pytan;
        stanowiska[stolik].pytania[idx] = pyt;
        stanowiska[stolik].liczba_zadanych_pytan++;
        egzamin->lista[id_kandydata_PD].pytania[idx] = pyt;
        egzamin->lista[id_kandydata_PD].id_egzaminatora[idx] = P_id;
        
        dodaj_do_loggera(plik_logu, "[Komisja B] [PID: %d] |\t Pytanie nr: %d |\t Egzaminator [P] [TID: %ld] [ID: %d] |\t Zadał kandydatowi [PID: %d] treść nr: %d\n",
            (int)getpid(),
            idx + 1,
            (long)syscall(SYS_gettid),
            P_id,
            kandydat_pid,
            pyt);

        pthread_mutex_unlock(&stanowiska[stolik].mutex);

        pthread_barrier_wait(&stanowiska[stolik].bariera);
        
        pthread_mutex_lock(&egzamin->lista[id_kandydata_PD].mutex_ipc);
        egzamin->lista[id_kandydata_PD].status_arkusza = 1;
        pthread_cond_signal(&egzamin->lista[id_kandydata_PD].cond_ipc);
        
        while (egzamin->lista[id_kandydata_PD].status_arkusza != 2) {
            pthread_cond_wait(&egzamin->lista[id_kandydata_PD].cond_ipc, &egzamin->lista[id_kandydata_PD].mutex_ipc);
        }
        pthread_mutex_unlock(&egzamin->lista[id_kandydata_PD].mutex_ipc);

        pthread_barrier_wait(&stanowiska[stolik].bariera);

        Student *s = &egzamin->lista[id_kandydata_PD];
        for (int k = 0; k < 5; k++) {
            if (s->id_egzaminatora[k] == P_id && s->oceny[k] == -1) {
                int ocena = (rand() % 100);
                s->oceny[k] = ocena;
                
                dodaj_do_loggera(plik_logu, "[Komisja B] [PID: %d] |\t Egzaminator [P] [TID: %ld] [ID: %d] |\t Ocenił kandydata [PID: %d] |\t Wynik cząstkowy: %d\n",
                       (int)getpid(),
                       (long)syscall(SYS_gettid),
                       P_id,
                       s->id,
                       ocena);
            }
        }

        pthread_barrier_wait(&stanowiska[stolik].bariera);

        int suma = 0;
        for (int k = 0; k < LICZBA_EGZAMINATOROW_B; k++) {
            suma += egzamin->lista[id_kandydata_PD].oceny[k];
        }
        double srednia = round(((double)suma / LICZBA_EGZAMINATOROW_B) * 100.0) / 100.0;
        egzamin->lista[id_kandydata_PD].punkty_praktyka = srednia;
        egzamin->lista[id_kandydata_PD].status = 3; 
        
        pthread_mutex_lock(&egzamin->lista[id_kandydata_PD].mutex_ipc);
        if (srednia >= 30.0) {
            egzamin->lista[id_kandydata_PD].zaliczona_B = 1; 
            dodaj_do_loggera(plik_logu, "[Komisja B] [PID: %d] |\t Kandydat [PID: %d] |\t ZDAŁ B |\t Wynik: %.2lf pkt\n", 
                   (int)getpid(), stanowiska[stolik].id_kandydata, srednia);
                   
        } else {
            egzamin->lista[id_kandydata_PD].zaliczona_B = 0; 
            dodaj_do_loggera(plik_logu, "[Komisja B] [PID: %d] |\t Kandydat [PID: %d] |\t OBLAŁ B |\t Wynik: %.2lf pkt\n", 
                   (int)getpid(), stanowiska[stolik].id_kandydata, srednia);
        }
        pthread_cond_signal(&egzamin->lista[id_kandydata_PD].cond_ipc);
        pthread_mutex_unlock(&egzamin->lista[id_kandydata_PD].mutex_ipc);
        
        stanowiska[stolik].zajete = 0;
        sem_post(sem_miejsca_komisji);
    }
    return NULL;
}

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
    }
    return NULL;
}

int main(int argc, char* argv[]){
    srand(time(NULL) ^ getpid());

    if (argc < 2) {
        return 1;
    }
    
    char nazwa_pliku_logu[32];
    snprintf(nazwa_pliku_logu, sizeof(nazwa_pliku_logu), "komisja_%c", argv[1][0]);
    plik_logu = otworz_log(nazwa_pliku_logu, "w");

    char typ_komisji = argv[1][0];
    int liczba_czlonkow = 0;
    pthread_t *egzaminatorzy;

    for (int i = 0; i < MAX_MIEJSC; i++) {
        stanowiska[i].zajete = 0;
        stanowiska[i].potrzebni_czlonkowie = 0;
        pthread_mutex_init(&stanowiska[i].mutex, NULL);
        
        int n = (typ_komisji == 'A') ? LICZBA_EGZAMINATOROW_A : LICZBA_EGZAMINATOROW_B;
        pthread_barrier_init(&stanowiska[i].bariera, NULL, n);
    }

    int shm_fd = shm_open(SHM_NAME,O_RDWR,0600);
    if (shm_fd == -1) {
        if (plik_logu) fclose(plik_logu);
        return 4;
    }
    egzamin = mmap(NULL,sizeof(EgzaminPamiecDzielona), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    
    if (typ_komisji == 'A'){
        sem_kolejka_komisji = sem_open(KOLEJKA_KOMISJA_A,0);
        sem_miejsca_komisji = sem_open(WOLNE_MIEJSCA_KOMISJA_A,0);
        sem_kolejka_przyszlosci = sem_open(KOLEJKA_KOMISJA_B,0);
    }else{
        sem_kolejka_komisji = sem_open(KOLEJKA_KOMISJA_B,0);
        sem_miejsca_komisji = sem_open(WOLNE_MIEJSCA_KOMISJA_B,0);
        sem_kolejka_przyszlosci = NULL; 
    }

    if (sem_kolejka_komisji == SEM_FAILED || sem_miejsca_komisji == SEM_FAILED) {
        if (plik_logu) fclose(plik_logu);
        return 6;
    }

    if (typ_komisji == 'A') {
        liczba_czlonkow = LICZBA_EGZAMINATOROW_A;
        egzaminatorzy = malloc(sizeof(pthread_t) * liczba_czlonkow);
        for (int i = 0; i < LICZBA_EGZAMINATOROW_A; i++){
            int *numer_egzaminujacego = malloc(sizeof(int));
            *numer_egzaminujacego = i;
            if (i == 0) {
                pthread_create(&egzaminatorzy[i], NULL, &przewodniczacy_komisji_A, numer_egzaminujacego);
            } else {
                pthread_create(&egzaminatorzy[i], NULL, &czlonek_komisji_A, numer_egzaminujacego);
            }
        }
    } else {
        liczba_czlonkow = LICZBA_EGZAMINATOROW_B;
        egzaminatorzy = malloc(sizeof(pthread_t) * liczba_czlonkow);
        for (int i = 0; i < LICZBA_EGZAMINATOROW_B; i++){
            int *numer_egzaminujacego = malloc(sizeof(int));
            *numer_egzaminujacego = i;
            if (i == 0) {
                pthread_create(&egzaminatorzy[i], NULL, &przewodniczacy_komisji_B, numer_egzaminujacego);
            } else {
                pthread_create(&egzaminatorzy[i], NULL, &czlonek_komisji_B, numer_egzaminujacego);
            }
        }
    }

    for (int i = 0; i < liczba_czlonkow; i++) {
        pthread_join(egzaminatorzy[i], NULL);
    }
    
    free(egzaminatorzy); 
    sem_close(sem_kolejka_komisji);
    sem_close(sem_miejsca_komisji);
    if (sem_kolejka_przyszlosci != NULL) {
        sem_close(sem_kolejka_przyszlosci);
    }

    if (plik_logu) fclose(plik_logu);
    return 0;
}