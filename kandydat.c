#include "common.h"

int main(int argc, char *argv[]) {
    const char* nazwa_pliku = (argc > 1) ? argv[1] : "kandydat_error";
    FILE *plik_logu = otworz_log(nazwa_pliku, "a");

    sem_t *sem_sync = sem_open(SEM_SYNC_START, 0);

    srand(getpid()); 
    
    Zgloszenie zgloszenie_kandydat;
    zgloszenie_kandydat.id = getpid();
    
    if ((rand() % 100) < SZANSA_NA_ZDANA_TEORIE) {
        zgloszenie_kandydat.zdana_matura = 1;       
        zgloszenie_kandydat.zdana_teoria_wczesniej = 1; 
    } else {
        zgloszenie_kandydat.zdana_teoria_wczesniej = 0; 
        if ((rand() % 100) < SZANSA_NA_BRAK_MATURY) {
            zgloszenie_kandydat.zdana_matura = 0; 
        } else {
            zgloszenie_kandydat.zdana_matura = 1;
        }
    }

    if (zgloszenie_kandydat.zdana_matura == 1 ){
        dodaj_do_loggera(plik_logu, "[Kandydat] [PID: %d]\t Zostałem utworzony i mam zdaną mature.\n",(int)getpid());
    }else{
        dodaj_do_loggera(plik_logu, "[Kandydat] [PID: %d]\t Zostałem utworzony i nie mam zdanej matury.\n",(int)getpid());
    }
    
    int file_descriptor = open(FIFO_WEJSCIE, O_WRONLY);
    if (file_descriptor == -1) {
        perror("[Kandydat] Nie mogę wejść do kolejki"); 

        if (sem_sync) sem_post(sem_sync);
        if (sem_sync) sem_close(sem_sync);

        if (plik_logu) fclose(plik_logu);
        return 1;
    }

    if (write(file_descriptor, &zgloszenie_kandydat, sizeof(Zgloszenie)) == -1) {
        perror("[Kandydat] Błąd wysyłania zgłoszenia");
        
        if (sem_sync) sem_post(sem_sync);
        if (sem_sync) sem_close(sem_sync);
        
        if (plik_logu) fclose(plik_logu);
        return 2;
    }
    close(file_descriptor);

    if (sem_sync) {
        sem_post(sem_sync);
        sem_close(sem_sync);
    }

    if (zgloszenie_kandydat.zdana_matura == 0) {
        if (plik_logu) fclose(plik_logu);
        return 0;
    }

    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0600);
    if (shm_fd == -1) {
        perror("[Kandydat] Błąd utowrzenia shm_fd (identyfikator)");
        if (plik_logu) fclose(plik_logu);
        return 1; 
    }
    
    EgzaminPamiecDzielona *egzamin = mmap(NULL, sizeof(EgzaminPamiecDzielona), 
    PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (egzamin == MAP_FAILED) {
        perror("[Kandydat] Błąd odblokowania pamieci dzielonej");
        if (plik_logu) fclose(plik_logu);
        return 1;
    }

    int moj_index = -1;

    // --- BEZPIECZNE OCZEKIWANIE NA WPISANIE NA LISTĘ ---
    pthread_mutex_lock(&egzamin->mutex_rejestracji);
    while (moj_index == -1) {
        for (int i = 0; i < egzamin->liczba_kandydatow; i++) {
            if (egzamin->lista[i].id == getpid()) {
                moj_index = i;
                break;
            }
        }
        if (moj_index == -1) {
            pthread_cond_wait(&egzamin->cond_rejestracji, &egzamin->mutex_rejestracji);
        }
    }
    pthread_mutex_unlock(&egzamin->mutex_rejestracji);
    // ---------------------------------------------------

    Student *kandydat = &egzamin->lista[moj_index];

    while (1) {
        pthread_mutex_lock(&kandydat->mutex_ipc);

        while (kandydat->status_arkusza != 1 && kandydat->status != 3) {
            pthread_cond_wait(&kandydat->cond_ipc, &kandydat->mutex_ipc);
        }

        if (kandydat->status == 3) {
            pthread_mutex_unlock(&kandydat->mutex_ipc);
            break;
        }

        if (kandydat->status_arkusza == 1) {
            int aktualny_status = kandydat->status;
            int limit_pytan = 0;

            if (aktualny_status == 1 || aktualny_status == 11) {
                limit_pytan = LICZBA_EGZAMINATOROW_A;
            } else if (aktualny_status == 2 || aktualny_status == 22) {
                limit_pytan = LICZBA_EGZAMINATOROW_B;
            }

            dodaj_do_loggera(plik_logu, "[Kandydat] [PID: %d] \t Otrzymałem %d pytań! Odpowiadam...\n", getpid(), limit_pytan);
            
            sleep_ms(GODZINA_Ti); 

            for (int i = 0; i < limit_pytan; i++) {
                int pytanie = kandydat->pytania[i];
                int odpowiedz = pytanie + (rand() % 10) + 1; 
                kandydat->odpowiedzi[i] = odpowiedz;
            }

            kandydat->status_arkusza = 2; // Gotowe
            pthread_cond_signal(&kandydat->cond_ipc); // Budzimy komisję
        }

        pthread_mutex_unlock(&kandydat->mutex_ipc);
    }

    if (plik_logu) fclose(plik_logu);
    return 0;
}