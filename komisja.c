#include "common.h"
#include <pthread.h>
#include <string.h>
#include <semaphore.h> 
#include <sys/mman.h>
#include <fcntl.h>     
#include <unistd.h> 

EgzaminPamiecDzielona *egzamin;
Stanowisko stanowiska[MAX_MIEJSC]; 
pthread_mutex_t mutex_szukania = PTHREAD_MUTEX_INITIALIZER; 

sem_t *sem_kolejka_komisji;
sem_t *sem_miejsca_komisji;
sem_t *sem_kolejka_przyszlosci; 

int znajdz_wolne_stanowisko() {
    int znaleziony = -1;
    pthread_mutex_lock(&mutex_szukania);
    for(int i=0; i<MAX_MIEJSC; i++) {
        if(stanowiska[i].zajete == 0) {
            stanowiska[i].zajete = 1; 
            znaleziony = i;
            break;
        }
    }
    pthread_mutex_unlock(&mutex_szukania);
    return znaleziony;
}

int wylosuj_unikalne_pytanie(int nr_stanowiska) {
    int unikalne = 0;
    int pytanie;
    while (!unikalne) {
        pytanie = (rand() % 50) + 1;
        unikalne = 1;
        for (int i = 0; i < stanowiska[nr_stanowiska].liczba_zadanych_pytan; i++) {
            if (stanowiska[nr_stanowiska].pytania[i] == pytanie) {
                unikalne = 0; 
                break;
            }
        }
    }
    return pytanie;
}

// ------------------- KOMISJA A -------------------

void* przewodniczacy_komisji_A(void* arg){
    int P_id = *(int*)arg;
    free(arg); 
    printf("[Komisja] [P] Przewodniczący %d w komisji A: Rozpoczynam pracę.\n", P_id);

    while(1) {
        sem_wait(sem_kolejka_komisji); 
        sem_wait(sem_miejsca_komisji); 

        int stolik = znajdz_wolne_stanowisko();
        
        pthread_mutex_lock(&stanowiska[stolik].mutex);
        int id_kandydata_PD = -1;
        int kandydat_pid = -1;

        for(int i=0; i<egzamin->liczba_kandydatow; i++) {
            if(egzamin->lista[i].status == 1) { 
                egzamin->lista[i].status = 11;  
                id_kandydata_PD = i;
                kandydat_pid = egzamin->lista[i].id;
                break;
            }
        }
        // jesli kandydata nie znajdzie zwalniamy stolik w sali
        if (id_kandydata_PD == -1) {
            stanowiska[stolik].zajete = 0;
            stanowiska[stolik].gotowe = 0;
            pthread_mutex_unlock(&stanowiska[stolik].mutex);
            sem_post(sem_miejsca_komisji);
            continue; 
        }

        stanowiska[stolik].id_kandydata = kandydat_pid;
        stanowiska[stolik].liczba_zadanych_pytan = 0;
        stanowiska[stolik].suma_ocen = 0;
        stanowiska[stolik].gotowe = 0;
        for(int i=0; i<5; i++) stanowiska[stolik].kto_pytal[i] = 0;

        stanowiska[stolik].gotowe = 1; 

        pthread_mutex_unlock(&stanowiska[stolik].mutex);

        while(1) {
             pthread_mutex_lock(&stanowiska[stolik].mutex);
             if (stanowiska[stolik].liczba_zadanych_pytan == LICZBA_EGZAMINATOROW_A - 1) {
                 pthread_mutex_unlock(&stanowiska[stolik].mutex);
                 break; 
             }
             pthread_mutex_unlock(&stanowiska[stolik].mutex);
             usleep(100); 
        }

        pthread_mutex_lock(&stanowiska[stolik].mutex);
        
        int pyt = wylosuj_unikalne_pytanie(stolik);
        stanowiska[stolik].pytania[stanowiska[stolik].liczba_zadanych_pytan++] = pyt;
        stanowiska[stolik].suma_ocen += (rand() % 101);
        stanowiska[stolik].kto_pytal[P_id] = 1; 

        printf("[Komisja] Egzaminator [P] %d w komisji A zadał %d. pytanie (z bazy: %d) kandydatowi PID: %d\n", 
               P_id, 
               stanowiska[stolik].liczba_zadanych_pytan,
               pyt,
               stanowiska[stolik].id_kandydata);
        
        pthread_mutex_unlock(&stanowiska[stolik].mutex);

        usleep(GODZINA_Ti); // 

        // ODPOWIEDZ NA PYTANIE KANDYDATA

        pthread_mutex_lock(&stanowiska[stolik].mutex);
        
        if(id_kandydata_PD != -1) {
            double srednia = (double)stanowiska[stolik].suma_ocen / LICZBA_EGZAMINATOROW_A;
            egzamin->lista[id_kandydata_PD].punkty_teoria = (int)srednia;
            
            printf("[Komisja] KANDYDAT %d ukonczył egzamin A z wynikiem %d\n", 
                   stanowiska[stolik].id_kandydata, (int)srednia);

            if (srednia >= 30.0) {
                egzamin->lista[id_kandydata_PD].status = 2; // Idzie do B
                sem_post(sem_kolejka_przyszlosci);
            } else {
                egzamin->lista[id_kandydata_PD].status = 3; // Odpada
            }
        }
        
        stanowiska[stolik].zajete = 0; 
        stanowiska[stolik].gotowe = 0; 
        pthread_mutex_unlock(&stanowiska[stolik].mutex);
        
        sem_post(sem_miejsca_komisji); 
    }
    return NULL;
}

void* czlonek_komisji_A(void* arg){
    int id = *(int*)arg;
    free(arg);
    
    while(1) {
        for(int i=0; i<MAX_MIEJSC; i++) {
            pthread_mutex_lock(&stanowiska[i].mutex);
            
            if (stanowiska[i].zajete == 1 && 
                stanowiska[i].gotowe == 1 && 
                stanowiska[i].kto_pytal[id] == 0 && 
                stanowiska[i].liczba_zadanych_pytan < LICZBA_EGZAMINATOROW_A - 1) {
                
                int pyt = wylosuj_unikalne_pytanie(i);
                stanowiska[i].pytania[stanowiska[i].liczba_zadanych_pytan++] = pyt;
                // 

                stanowiska[i].suma_ocen += (rand() % 101);
                stanowiska[i].kto_pytal[id] = 1; 

                printf("[Komisja] Egzaminator [C] %d w komisji A zadał %d. pytanie (z bazy: %d) kandydatowi PID: %d\n", 
                       id, 
                       stanowiska[i].liczba_zadanych_pytan,
                       pyt, 
                       stanowiska[i].id_kandydata);
            }
            pthread_mutex_unlock(&stanowiska[i].mutex);
        }
        usleep(2000); 
    }
    return NULL;
}

// ------------------- KOMISJA B -------------------

void* przewodniczacy_komisji_B(void* arg){
    int P_id = *(int*)arg;
    free(arg); 
    printf("[Komisja] [P] Przewodniczący %d w komisji B: Rozpoczynam pracę.\n", P_id);

    while(1) {
        sem_wait(sem_kolejka_komisji); 
        sem_wait(sem_miejsca_komisji);

        int stolik = znajdz_wolne_stanowisko();
        
        pthread_mutex_lock(&stanowiska[stolik].mutex);
        int id_kandydata_PD = -1;
        int kandydat_pid = -1;

        for(int i=0; i<egzamin->liczba_kandydatow; i++) {
            if(egzamin->lista[i].status == 2) { 
                egzamin->lista[i].status = 22;  
                id_kandydata_PD = i;
                kandydat_pid = egzamin->lista[i].id;
                break;
            }
        }

        if (id_kandydata_PD == -1) {
            stanowiska[stolik].zajete = 0;
            stanowiska[stolik].gotowe = 0;
            pthread_mutex_unlock(&stanowiska[stolik].mutex);
            sem_post(sem_miejsca_komisji);
            continue;
        }

        stanowiska[stolik].id_kandydata = kandydat_pid;
        stanowiska[stolik].liczba_zadanych_pytan = 0;
        stanowiska[stolik].suma_ocen = 0;
        stanowiska[stolik].gotowe = 0;
        for(int i=0; i<5; i++) {
            stanowiska[stolik].kto_pytal[i] = 0; 
        }
        stanowiska[stolik].gotowe = 1;

        pthread_mutex_unlock(&stanowiska[stolik].mutex);

        while(1) {
             pthread_mutex_lock(&stanowiska[stolik].mutex);
             if (stanowiska[stolik].liczba_zadanych_pytan == LICZBA_EGZAMINATOROW_B - 1) {
                 pthread_mutex_unlock(&stanowiska[stolik].mutex);
                 break; 
             }
             pthread_mutex_unlock(&stanowiska[stolik].mutex);
             usleep(100);
        }

        pthread_mutex_lock(&stanowiska[stolik].mutex);
        int pyt = wylosuj_unikalne_pytanie(stolik);
        stanowiska[stolik].pytania[stanowiska[stolik].liczba_zadanych_pytan++] = pyt;
        stanowiska[stolik].suma_ocen += (rand() % 101);
        stanowiska[stolik].kto_pytal[P_id] = 1;
        
        printf("[Komisja] Egzaminator [P] %d w komisji B zadał %d. pytanie (z bazy: %d) kandydatowi PID: %d\n", 
               P_id, 
               stanowiska[stolik].liczba_zadanych_pytan,
               pyt, 
               stanowiska[stolik].id_kandydata);
        pthread_mutex_unlock(&stanowiska[stolik].mutex);

        usleep(GODZINA_Ti); // czas na odpowiedz kandydata

        pthread_mutex_lock(&stanowiska[stolik].mutex);
        if(id_kandydata_PD != -1) {
            double srednia = (double)stanowiska[stolik].suma_ocen / LICZBA_EGZAMINATOROW_B;
            egzamin->lista[id_kandydata_PD].punkty_praktyka = (int)srednia;
            egzamin->lista[id_kandydata_PD].status = 3; 
            
            printf("[Komisja] KANDYDAT %d ukonczył egzamin B z wynikiem %d\n", 
                   stanowiska[stolik].id_kandydata, (int)srednia);
        }
        
        stanowiska[stolik].zajete = 0;
        stanowiska[stolik].gotowe = 0;
        pthread_mutex_unlock(&stanowiska[stolik].mutex);
        sem_post(sem_miejsca_komisji);
    }
    return NULL;
}

void* czlonek_komisji_B(void* arg){
    int id = *(int*)arg;
    free(arg);
    
    while(1) {
        for(int i=0; i<MAX_MIEJSC; i++) {
            pthread_mutex_lock(&stanowiska[i].mutex);
            
            if (stanowiska[i].zajete == 1 && 
                stanowiska[i].gotowe == 1 &&
                stanowiska[i].kto_pytal[id] == 0 &&
                stanowiska[i].liczba_zadanych_pytan < LICZBA_EGZAMINATOROW_B - 1) {
                
                int pyt = wylosuj_unikalne_pytanie(i);
                stanowiska[i].pytania[stanowiska[i].liczba_zadanych_pytan++] = pyt;
                stanowiska[i].suma_ocen += (rand() % 101);
                stanowiska[i].kto_pytal[id] = 1; 

                printf("[Komisja] Egzaminator [C] %d w komisji B zadał %d. pytanie (z bazy: %d) kandydatowi PID: %d\n", 
                       id, 
                       stanowiska[i].liczba_zadanych_pytan,
                       pyt, 
                       stanowiska[i].id_kandydata);
            }
            pthread_mutex_unlock(&stanowiska[i].mutex);
        }
        usleep(2000);
    }
    return NULL;
}

// ------------------- MAIN -------------------

int main(int argc, char* argv[]){

    if (argc < 2) {
        fprintf(stderr, "[Komisja] Brak argumentu typu komisji!\n");
        return 1;
    }

    char typ_komisji = argv[1][0];
    int liczba_czlonkow = 0;
    pthread_t *egzaminatorzy;

    // Inicjalizacja stolików 
    for(int i=0; i<MAX_MIEJSC; i++) {
        stanowiska[i].zajete = 0;
        stanowiska[i].gotowe = 0;
        pthread_mutex_init(&stanowiska[i].mutex, NULL);
        pthread_cond_init(&stanowiska[i].cond_bariera, NULL);
    }

    int shm_fd = shm_open(SHM_NAME,O_RDWR,0600);
    if (shm_fd == -1){
        perror("[Komisja]: Bład shm_open\n");
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
        perror("[Komisja] Błąd otwarcia semaforów");
        return 6;
    }

    // Tworzenie watków
    if (typ_komisji == 'A')
    {
        liczba_czlonkow = LICZBA_EGZAMINATOROW_A;
        egzaminatorzy = malloc(sizeof(pthread_t) * liczba_czlonkow);

        for (int i=0; i< LICZBA_EGZAMINATOROW_A; i++){
            int *numer_egzaminujacego = malloc(sizeof(int));
            *numer_egzaminujacego = i;
            
            if (i == 0){            
                if (pthread_create(&egzaminatorzy[i], NULL, &przewodniczacy_komisji_A, numer_egzaminujacego) != 0) return 4;
            }else{
                if (pthread_create(&egzaminatorzy[i], NULL, &czlonek_komisji_A, numer_egzaminujacego) != 0) return 4;
            }
        }
    } else if (typ_komisji == 'B')
    {
        liczba_czlonkow = LICZBA_EGZAMINATOROW_B;
        egzaminatorzy = malloc(sizeof(pthread_t) * liczba_czlonkow);

        for (int i=0; i< LICZBA_EGZAMINATOROW_B; i++){
            int *numer_egzaminujacego = malloc(sizeof(int));
            *numer_egzaminujacego = i;
            
            if (i == 0){            
                if (pthread_create(&egzaminatorzy[i], NULL, &przewodniczacy_komisji_B, numer_egzaminujacego) != 0) return 4;
            }else{
                if (pthread_create(&egzaminatorzy[i], NULL, &czlonek_komisji_B, numer_egzaminujacego) != 0) return 4;
            }
        }
    } else {
        fprintf(stderr, "[Komisja] Nieznana komisja: %c.\n", typ_komisji);
        return 5;
    }

    for (int i = 0; i < liczba_czlonkow; i++) {
        pthread_join(egzaminatorzy[i], NULL);
    }

    free(egzaminatorzy); 
    sem_close(sem_kolejka_komisji);
    sem_close(sem_miejsca_komisji);
    if (sem_kolejka_przyszlosci != NULL) sem_close(sem_kolejka_przyszlosci);
    
    return 0;
}