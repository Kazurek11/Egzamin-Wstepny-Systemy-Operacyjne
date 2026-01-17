#include "common.h"

int main(int argc, char *argv[]) {
    const char* nazwa_pliku = (argc > 1) ? argv[1] : "kandydat_error";
    FILE *plik_logu = otworz_log(nazwa_pliku, "a");
    AKTUALNY_KOLOR_LOGU = ANSI_COLOR_BLUE;

    sem_t *sem_sync = sem_open(SEM_SYNC_START, 0);


    srand(getpid()); 
    
    Zgloszenie zgloszenie_kandydat;
    zgloszenie_kandydat.id = getpid();
    
    // Losujemy statystyki kandydata (czy ma maturę, czy zdał teorię wcześniej)
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
    
    // Wysyłamy zgłoszenie do Dziekana przez kolejkę FIFO
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

    // Jeśli kandydat nie ma matury, kończy działanie od razu (nie wchodzi do systemu)
    if (zgloszenie_kandydat.zdana_matura == 0) {
        if (plik_logu) fclose(plik_logu);
        return 0;
    }

    // Podłączamy się do pamięci dzielonej, żeby widzieć egzamin
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
    // Czekamy, aż Dziekan przetworzy zgłoszenie i wpisze nas do pamięci dzielonej
    pthread_mutex_lock(&egzamin->mutex_rejestracji);
    while (moj_index == -1) {
        for (int i = 0; i < egzamin->liczba_kandydatow; i++) {
            if (egzamin->lista[i].id == getpid()) {
                moj_index = i;
                break;
            }
        }
        if (moj_index == -1) {
            // Usypiamy proces do momentu, aż Dziekan zaktualizuje listę
            pthread_cond_wait(&egzamin->cond_rejestracji, &egzamin->mutex_rejestracji);
        }
    }
    pthread_mutex_unlock(&egzamin->mutex_rejestracji);
    // ---------------------------------------------------
    
    Student *kandydat = &egzamin->lista[moj_index];

    while (1) {
        // Blokujemy dostęp do własnych danych, żeby sprawdzić status
        pthread_mutex_lock(&kandydat->mutex_ipc);

        // Czekamy na sygnał od Komisji: 1 (masz pytania) lub 3 (koniec/odrzucenie)
        while (kandydat->status_arkusza != 1 && kandydat->status != 3) {
            pthread_cond_wait(&kandydat->cond_ipc, &kandydat->mutex_ipc);
        }

        // Jeśli status to 3, oznacza to koniec egzaminu dla tego kandydata
        if (kandydat->status == 3) {
            pthread_mutex_unlock(&kandydat->mutex_ipc);
            break;
        }

        // Jeśli otrzymaliśmy arkusz z pytaniami (status 1)
        if (kandydat->status_arkusza == 1) {
            int limit_pytan = 0;

            // Sprawdzamy ile pytań faktycznie dostaliśmy
            for (int i = 0; i < 5; i++) {
                if (kandydat->pytania[i] > 0) {
                    limit_pytan++;
                }
            }

            dodaj_do_loggera(plik_logu, "[Kandydat] [PID: %d] \t Otrzymałem %d pytań! Odpowiadam...\n", getpid(), limit_pytan);
            
            sleep_ms(GODZINA_Ti); 

            // Definiujemy typ komisji na podstawie statusu (przed pętlą)
            char typ_komisji = (kandydat->status == 11 || kandydat->status == 1) ? 'A' : 'B';

            // Udzielamy losowych odpowiedzi na otrzymane pytania
            for (int i = 0; i < 5; i++) {
                if (kandydat->pytania[i] > 0) {
                    // Pobieramy dane z pamięci dzielonej
                    int pytanie = kandydat->pytania[i];
                    int id_egzaminatora = kandydat->id_egzaminatora[i];
                    
                    // Określamy typ egzaminatora (0 = Przewodniczący, inni = Członkowie)
                    char typ_egzaminatora = (id_egzaminatora == 0) ? 'P' : 'C';

                    int odpowiedz = pytanie + (rand() % 10) + 1; 
                    kandydat->odpowiedzi[i] = odpowiedz;

                    dodaj_do_loggera(plik_logu, 
                        "[Kandydat] [PID: %d] \t [Komisja %c] Pytanie: %d o numerze %d od Egzaminatora [%c] [ID: %d] \t Moja odpowiedź to: %d.\n", 
                        getpid(),
                        typ_komisji, 
                        pytanie, 
                        i + 1,
                        typ_egzaminatora,
                        id_egzaminatora,
                        odpowiedz);
                }
            }

            // Oznaczamy arkusz jako wypełniony (status 2) i budzimy Komisję
            kandydat->status_arkusza = 2; // Gotowe
            pthread_cond_signal(&kandydat->cond_ipc); // Budzimy komisję
        }

        pthread_mutex_unlock(&kandydat->mutex_ipc);
    }

    if (plik_logu) fclose(plik_logu);
    return 0;
}