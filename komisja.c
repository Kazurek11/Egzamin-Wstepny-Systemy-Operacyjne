#include "common.h"
#include <pthread.h>
#include <string.h>

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
        fprintf(stderr, "Brak argumentu typu komisji!\n");
        return 1;
    }

    char typ_komisji = argv[1][0];
    int liczba_czlonkow = 0;
    pthread_t *egzaminatorzy; 

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
    return 0;
}