#include "common.h"
#include <sys/wait.h>
#include <sys/mman.h> 
#include <semaphore.h>
#include <errno.h>

int main() {
    srand(time(NULL) ^ getpid());

    if (unlink(FIFO_WEJSCIE) == -1) {
        if (errno != ENOENT) { 
            perror("[Dziekan] Błąd unlink (sprzątanie FIFO)");
        }
    }

    if (mkfifo(FIFO_WEJSCIE, 0600) == -1) {
        perror("[Dziekan] Błąd mkfifo");
        return 1;
    }

    if (shm_unlink(SHM_NAME) == -1) {
        if (errno != ENOENT) {
             perror("[Dziekan] Ostrzeżenie: Nie udało się usunąć starej pamięci dzielonej");
        }
    }

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
    if (shm_fd == -1) {
        perror("[Dziekan] Błąd krytyczny: shm_open");
        unlink(FIFO_WEJSCIE);
        return 4;
    }

    if (ftruncate(shm_fd, sizeof(EgzaminPamiecDzielona)) == -1) {
        perror("[Dziekan] Błąd krytyczny: ftruncate");
        shm_unlink(SHM_NAME);
        unlink(FIFO_WEJSCIE);
        return 4;
    }

    EgzaminPamiecDzielona *egzamin = mmap(NULL, sizeof(EgzaminPamiecDzielona), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (egzamin == MAP_FAILED) {
        perror("[Dziekan] Błąd krytyczny: mmap");
        shm_unlink(SHM_NAME);
        unlink(FIFO_WEJSCIE);
        return 4;
    }
    // Na poczatku nikt nie czeka do kolejki bo komisja siedzi w sali a dziekan jeszcze nikogo nie wpuscił
    sem_t *sem_kolejka_A = sem_open(KOLEJKA_KOMISJA_A,O_CREAT,0600,0);
    sem_t *sem_kolejka_B = sem_open(KOLEJKA_KOMISJA_B,O_CREAT,0600,0);
    // Trzy miejsca w sali czekaja na kandydatów wiec startujemy od 3.
    sem_t *sem_miejsce_A = sem_open(WOLNE_MIEJSCA_KOMISJA_A,O_CREAT,0600,3);
    sem_t *sem_miejsce_B = sem_open(WOLNE_MIEJSCA_KOMISJA_B,O_CREAT,0600,3);
    
    if (sem_kolejka_A == SEM_FAILED){
        perror("[Dziekan] Bład otwarcia sem_kolejka_A");
        return 6;
    }
    if (sem_kolejka_B == SEM_FAILED){
        perror("[Dziekan] Bład otwarcia sem_kolejka_B");
        return 6;
    }
    if (sem_miejsce_A == SEM_FAILED){
        perror("[Dziekan] Bład otwarcia sem_miejsce_A");
        return 6;
    }
    if (sem_miejsce_B == SEM_FAILED){
        perror("[Dziekan] Bład otwarcia sem_miejsce_B");
        return 6;
    }
    // Zamykam semafory bo sa one dla komisji
    // sem_close(sem_kolejka_A);
    // sem_close(sem_kolejka_B);
    sem_close(sem_miejsce_A);
    sem_close(sem_miejsce_B);

    // Inicjalizacja licznika w pamięci
    egzamin->liczba_kandydatow = 0;

    printf("[Dziekan] Rekrutacja otwarta. Generuję komisje i kandydatów \n");

    // ----------------- komisja --------------------
    for (int i = 0; i < LICZBA_KOMISJI; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("[Dziekan] Błąd fork (Komisja)");
            return 2;
        }
        
        if (pid == 0) {
            char *typ;
            if (i == 0) {
                typ = "A";
            }else{
                typ = "B";
            }
            execl("./komisja", "komisja", typ, NULL);
            
            perror("[Dziekan -> Komisja] Błąd execl");
            return 6; 
        }
    }

    // ----------------- kandydat --------------------
    for (int i = 0; i < LICZBA_KANDYDATOW; i++) {
        pid_t pid = fork();
        
        if (pid == -1) {
            perror("[Dziekan] Błąd fork (Kandydat)");
            unlink(FIFO_WEJSCIE); 
            return 2; 
        }

        if (pid == 0) {
            // --- PROCES DZIECKO (Kandydat) ---
            execl("./kandydat", "kandydat", NULL);
            perror("[Dziekan -> Kandydat] Błąd execl");
            return 6;
        }
        
        usleep(5000); // 5ms opóźnienia
    }

    printf("[Dziekan] Wszyscy kandydaci (%d) stoją przed wydziałem.\n", LICZBA_KANDYDATOW);
    printf("[Dziekan] Czekam na godzinę T (symulacja 1 sekunda)...\n");
    sleep(1); 

    printf("[Dziekan] Godzina T wybija! Otwieram drzwi (FIFO).\n");
    
    int fd = open(FIFO_WEJSCIE, O_RDONLY);
    if (fd == -1) {
        perror("[Dziekan] Błąd open FIFO");
        unlink(FIFO_WEJSCIE);
        return 3;
    }

    FILE *plik_przyjec = fopen("lista_przyjetych.txt", "w"); // log1
    FILE *plik_odrzucen = fopen("lista_odrzuconych.txt", "w"); // log2

    if (!plik_przyjec || !plik_odrzucen) {
        perror("[Dziekan] Błąd otwarcia plików raportowych"); // Kontynuujemy, ale wypiszemy błąd na stderr
        
    }

    Zgloszenie buf;
    int licznik_odrzuceni = 0;

    while (read(fd, &buf, sizeof(Zgloszenie)) > 0) {
        
        if (buf.zdana_matura == 1) {
            
            // --- ZAPIS DO PLIKU ---
            if (plik_przyjec) {
                fprintf(plik_przyjec, "PID: %d | Rekrutował wczesniej i zdał teorie: %s\n", 
                        buf.id, buf.zdana_teoria_wczesniej ? "TAK" : "NIE");
            }

            int idx = egzamin->liczba_kandydatow;
            
            if (idx < MAX_KANDYDATOW) {
                egzamin->lista[idx].id = buf.id;
                egzamin->lista[idx].zdana_matura = 1;
                egzamin->lista[idx].zdana_teoria_wczesniej = buf.zdana_teoria_wczesniej;
                
                egzamin->lista[idx].punkty_teoria = -1;
                egzamin->lista[idx].punkty_praktyka = -1;
                egzamin->lista[idx].czy_przyjety = 0;

                if (buf.zdana_teoria_wczesniej == 1) {
                    int ocena = (rand() % 71) + 30;
                    egzamin->lista[idx].status = 2; // Zdana teoria - wysyłamy na praktyke (komisja b)
                    sem_post(sem_kolejka_B);                    
                    egzamin->lista[idx].punkty_teoria = ocena; // Przepisana ocena z poprzedniego roku 
                    printf("[Dziekan] Kandydat %d (Stary) -> Skierowany na Praktykę.\n", buf.id);
                } else {
                    egzamin->lista[idx].status = 1; // Idzie na teorie (komisja a)
                    sem_post(sem_kolejka_A);
                    printf("[Dziekan] Kandydat %d (Nowy) -> Skierowany na Teorię.\n", buf.id);
                }

                egzamin->liczba_kandydatow++; 
            } else {
                fprintf(stderr, "[Dziekan] Błąd: Brak miejsca w pamięci dzielonej!\n");
            }

        } else {
            // --- KANDYDAT ODRZUCONY ---
            licznik_odrzuceni++;
            if (plik_odrzucen) {
                fprintf(plik_odrzucen, "PID: %d | Matura: %d | Powód: Brak matury\n", buf.id, buf.zdana_matura);
            }
            printf("[Dziekan] Kandydat %d ODRZUCONY (Brak matury).\n", buf.id);
        }
    }

    printf("[Dziekan] Zakończono przyjmowanie.\n");
    printf("[Dziekan] Na liste kandydatów (SHM) zapisano %d osób.\n", egzamin->liczba_kandydatow);
    printf("[Dziekan] Odrzucono (brak matury): %d osób.\n", licznik_odrzuceni);

    if (plik_przyjec) fclose(plik_przyjec);
    if (plik_odrzucen) fclose(plik_odrzucen);
    
    close(fd);
    unlink(FIFO_WEJSCIE);

    
    while (wait(NULL) > 0); // Czekamy na wszystkie procesy dzieci (Kandydaci + Komisje)

    
    shm_unlink(SHM_NAME); // Usuwanie pamięci dzielonej (zawsze na koncu!!)
    
    return 0;
}