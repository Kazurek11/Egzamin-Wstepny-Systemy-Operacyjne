#include "common.h"
#include <pthread.h>
#include <string.h>
#include <semaphore.h> // nowa biblioteka

EgzaminPamiecDzielona *egzamin;
pthread_mutex_t mutex_stali = PTHREAD_MUTEX_INITIALIZER; // Wystarczy 1 mutex na komisje (sale w ktorej pracuje komisja)

sem_t *sem_kolejka_komisji;
sem_t *sem_miejsca_komisji;
sem_t *sem_kolejka_przyszlosci; // Dla komisja A jest to semafor kontrolujacy gdzie pojdzie dalej student. Dla komisji B być moze wykorzystam to do informowania dziekana o koncowej ocenie kandydata?

void* przewodniczacy_komisji_A(void* arg){
    int P_id = *(int*)arg;
    free(arg); 
    printf("[Komisja A] Przewodniczący %d: Rozpoczynam pracę.\n", P_id);

    
    return NULL;
}

void* przewodniczacy_komisji_B(void* arg){
    int P_id = *(int*)arg;
    free(arg); 

    printf("[Komisja B] Przewodniczący %d: Rozpoczynam pracę.\n", P_id);
    
    return NULL;
}

void* czlonek_komisji_A(void* arg){
    int id = *(int*)arg;
    free(arg);

    printf("[Komisja A] Egzaminator %d: Gotowy.\n", id);
    
    return NULL;
}

void* czlonek_komisji_B(void* arg){
    int id = *(int*)arg;
    free(arg);

    printf("[Komisja B] Egzaminator %d: Gotowy.\n", id);
    
    return NULL;
}

int main(int argc, char* argv[]){

    if (argc < 2) {
        fprintf(stderr, "[Komisja] Brak argumentu typu komisji!\n");
        return 1;
    }

    char typ_komisji = argv[1][0];
    int liczba_czlonkow = 0;
    pthread_t *egzaminatorzy;

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
        sem_kolejka_przyszlosci = NULL; // tutaj moze dodam semafor dla dziekana w przyszlosci
    }

    if (sem_kolejka_komisji == SEM_FAILED) {
        perror("[Komisja] Błąd otwarcia semafora 'kolejka komisji' ");
        return 6;
    }

    if (sem_miejsca_komisji == SEM_FAILED) {
        perror("[Komisja] Błąd otwarcia semafora 'miejsca komisji' ");
        return 6;
    }

    if (sem_kolejka_przyszlosci != NULL){
        if (sem_kolejka_przyszlosci == SEM_FAILED){
            perror("[Komisja] Błąd otwarcia semafora 'miejsca komisji' ");
            return 6;
        }
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
                if (pthread_create(&egzaminatorzy[i], NULL, &przewodniczacy_komisji_A, numer_egzaminujacego) != 0){
                    fprintf(stderr, "[Komisja [A]] Błąd tworzenia wątku: %d: %s\n", *numer_egzaminujacego, strerror(errno));
                    return 4;
                }
            }else{
                if (pthread_create(&egzaminatorzy[i], NULL, &czlonek_komisji_A, numer_egzaminujacego) != 0){
                    fprintf(stderr, "[Komisja [A]] Błąd tworzenia wątku: %d: %s\n", *numer_egzaminujacego, strerror(errno));
                    return 4;
                }
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
                if (pthread_create(&egzaminatorzy[i], NULL, &przewodniczacy_komisji_B, numer_egzaminujacego) != 0){
                    fprintf(stderr, "[Komisja [B]] Błąd tworzenia wątku: %d: %s\n", *numer_egzaminujacego, strerror(errno));
                    return 4;
                }
            }else{
                if (pthread_create(&egzaminatorzy[i], NULL, &czlonek_komisji_B, numer_egzaminujacego) != 0){
                    fprintf(stderr, "[Komisja [B]] Błąd tworzenia wątku: %d: %s\n", *numer_egzaminujacego, strerror(errno));
                    return 4;
                }
            }
        }
    } else {
        fprintf(stderr, "[Komisja] Nieznana komisja: %c. %s\n", typ_komisji, strerror(errno));
        return 5;
    }

    for (int i = 0; i < liczba_czlonkow; i++) {
        pthread_join(egzaminatorzy[i], NULL);
    }

    free(egzaminatorzy); 
    sem_close(sem_kolejka_komisji);
    sem_close(sem_miejsca_komisji);

    if (sem_kolejka_przyszlosci != NULL){
        sem_close(sem_kolejka_przyszlosci);
    }
    return 0;
}