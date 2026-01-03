#include "common.h"

int main() {
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
        printf("[Kandydat] [PID: %d]\t Zostałem utworzony i mam zdaną mature.\n",(int)getpid());
    }else{
        printf("[Kandydat] [PID: %d]\t Zostałem utworzony i nie mam zdanej matury.\n",(int)getpid());
    }
    
    int file_descriptor = open(FIFO_WEJSCIE, O_WRONLY);
    if (file_descriptor == -1) {
        perror("[Kandydat] Nie mogę wejść do kolejki");
        return 1;
    }

    if (write(file_descriptor, &zgloszenie_kandydat, sizeof(Zgloszenie)) == -1) {
        perror("[Kandydat] Błąd wysyłania zgłoszenia");
        return 2;
    }
    close(file_descriptor);

    if (zgloszenie_kandydat.zdana_matura == 0) {
        return 0;
    }

    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0600);
    if (shm_fd == -1) {
        perror("[Kandydat] Błąd utowrzenia shm_fd (identyfikator)");
        return 1; 
    }
    
    EgzaminPamiecDzielona *egzamin = mmap(NULL, sizeof(EgzaminPamiecDzielona), 
    PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (egzamin == MAP_FAILED) {
        perror("[Kandydat] Błąd odblokowania pamieci dzielonej");
        return 1;
    }

    int moj_index = -1;
    int timeout = 0;
    // teamt do konsultacji nie usuwaj tego
    while (moj_index == -1 && timeout < 1000) {
        for (int i = 0; i < egzamin->liczba_kandydatow; i++) {
            if (egzamin->lista[i].id == getpid()) {
                moj_index = i;
                break;
            }
        }
        if (moj_index == -1) {
            sleep_ms(5); 
            timeout++;
        }
    }

    if (moj_index == -1) {
        return 0; 
    }
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

            printf("[Kandydat] [PID: %d] \t Otrzymałem %d pytań! Odpowiadam...\n", getpid(), limit_pytan);
            
            sleep_ms(GODZINA_Ti); 

            for (int i = 0; i < limit_pytan; i++) {
                int pytanie = kandydat->pytania[i];
                int odpowiedz = pytanie + (rand() % 10) + 1; 
                kandydat->odpowiedzi[i] = odpowiedz;
            }

            kandydat->status_arkusza = 2;
            pthread_cond_signal(&kandydat->cond_ipc);
        }

        pthread_mutex_unlock(&kandydat->mutex_ipc);
    }

    return 0;
}