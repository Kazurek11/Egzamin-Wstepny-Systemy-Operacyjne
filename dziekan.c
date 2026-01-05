#define _GNU_SOURCE
#include "common.h"

int main() {
    FILE *plik_logu = otworz_log("dziekan", "w");

    srand(time(NULL) ^ getpid());

    // --- SPRZĄTANIE ŚRODOWISKA ---
    sem_unlink(KOLEJKA_KOMISJA_A);
    sem_unlink(KOLEJKA_KOMISJA_B);
    sem_unlink(WOLNE_MIEJSCA_KOMISJA_A);
    sem_unlink(WOLNE_MIEJSCA_KOMISJA_B);
    sem_unlink(SEM_LOG_KEY);
    sem_unlink(SEM_SYNC_START); 

    if (unlink(FIFO_WEJSCIE) == -1) {
        if (errno != ENOENT) {
            perror("[Dziekan] Błąd unlink");
        }
    }
    mkfifo(FIFO_WEJSCIE, 0600);
    shm_unlink(SHM_NAME);

    int fd_fifo = open(FIFO_WEJSCIE, O_RDWR);
    if (fd_fifo == -1) {
        perror("[Dziekan] Błąd otwarcia FIFO");
        return 5;
    }

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
    if (shm_fd == -1) {
        if (plik_logu) fclose(plik_logu);
        return 4;
    }
    ftruncate(shm_fd, sizeof(EgzaminPamiecDzielona));
    
    EgzaminPamiecDzielona *egzamin = mmap(NULL, sizeof(EgzaminPamiecDzielona), 
                                          PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (egzamin == MAP_FAILED) {
        perror("[Dziekan] Błąd mmap");
        if (plik_logu) fclose(plik_logu);
        return 4;
    }
    
    pthread_mutexattr_t mattr;
    pthread_condattr_t cattr;
    
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    
    pthread_condattr_init(&cattr);
    pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);

    for (int i = 0; i < MAX_KANDYDATOW; i++) {
        pthread_mutex_init(&egzamin->lista[i].mutex_ipc, &mattr);
        pthread_cond_init(&egzamin->lista[i].cond_ipc, &cattr);
        egzamin->lista[i].status_arkusza = 0; 
    }

    pthread_mutexattr_destroy(&mattr);
    pthread_condattr_destroy(&cattr);

    sem_t *sem_kolejka_A = sem_open(KOLEJKA_KOMISJA_A, O_CREAT, 0600, 0);
    sem_t *sem_kolejka_B = sem_open(KOLEJKA_KOMISJA_B, O_CREAT, 0600, 0);
    sem_t *sem_miejsce_A = sem_open(WOLNE_MIEJSCA_KOMISJA_A, O_CREAT, 0600, 3);
    sem_t *sem_miejsce_B = sem_open(WOLNE_MIEJSCA_KOMISJA_B, O_CREAT, 0600, 3);
    
    sem_t *sem_sync = sem_open(SEM_SYNC_START, O_CREAT, 0600, 0);

    if (sem_kolejka_A == SEM_FAILED || sem_kolejka_B == SEM_FAILED || sem_sync == SEM_FAILED) {
        perror("[Dziekan] Błąd semaforow");
        if (plik_logu) fclose(plik_logu);
        return 6;
    }
    if (sem_miejsce_A == SEM_FAILED || sem_miejsce_B == SEM_FAILED) {
        perror("[Dziekan] Błąd semaforow wolnych miejsc");
        if (plik_logu) fclose(plik_logu);
        return 6;
    }
    
    sem_close(sem_miejsce_A);
    sem_close(sem_miejsce_B);

    egzamin->liczba_kandydatow = 0;

    dodaj_do_loggera(plik_logu, "[Dziekan] [PID: %d] Rekrutacja otwarta. Generuję komisje i kandydatów \n", (int)getpid());

    pid_t komisje_pids[LICZBA_KOMISJI];

    for (int i = 0; i < LICZBA_KOMISJI; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            char *typ = (i == 0) ? "A" : "B";
            execl("./komisja", "komisja", typ, NULL);
            return 6; 
        }
        komisje_pids[i] = pid; 
    }

    dodaj_do_loggera(plik_logu, "[Dziekan] [PID: %d] Rozpoczynam sekwencyjne tworzenie kandydatów...\n", (int)getpid());
    sleep_ms(5); // dodaje 5 ms dla poprawnego logu czas utworzenia pierwszego kandydata w testach był równy logowi
    // --- GENEROWANIE NAZWY PLIKU DLA KANDYDATÓW (Raz, na sztywno) ---
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char nazwa_dla_kandydatow[256];
    
    // Format: logi/ROK-MC-DZ_GO-MI-SE_logi_kandydaci.txt
    snprintf(nazwa_dla_kandydatow, sizeof(nazwa_dla_kandydatow), 
             "logi/%04d-%02d-%02d_%02d-%02d-%02d_logi_kandydaci.txt",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec);

    // Utwórz folder (jeśli nie istnieje), bo zaraz będziemy przekazywać ścieżkę
    utworz_folder("logi");

    for (int i = 0; i < LICZBA_KANDYDATOW; i++) {
        if (fork() == 0) {
            // Przekazujemy wygenerowaną nazwę pliku jako argument nr 1
            execl("./kandydat", "kandydat", nazwa_dla_kandydatow, NULL);
            return 6;
        }
        sem_wait(sem_sync);
    }
    
    sem_close(sem_sync);
    sem_unlink(SEM_SYNC_START);

    dodaj_do_loggera(plik_logu, "[Dziekan] [PID: %d] Wszyscy kandydaci (%d) czekają w kolejce FIFO.\n", (int)getpid(), LICZBA_KANDYDATOW);
    dodaj_do_loggera(plik_logu, "[Dziekan] [PID: %d] Czekam na godzinę T %d...\n", (int)getpid(), GODZINA_T);
    sleep(GODZINA_T); 
    dodaj_do_loggera(plik_logu, "[Dziekan] [PID: %d] Godzina T wybija! Odbieram zgłoszenia (FIFO).\n", (int)getpid());
    
    Zgloszenie buf;
    
    for (int i = 0; i < LICZBA_KANDYDATOW; i++) {
        if (read(fd_fifo, &buf, sizeof(Zgloszenie)) <= 0) {
            dodaj_do_loggera(plik_logu, "[Dziekan] Błąd lub przedwczesny koniec odczytu FIFO!\n");
            break;
        }

        int idx = egzamin->liczba_kandydatow;
        if (idx < MAX_KANDYDATOW) {
            egzamin->lista[idx].id = buf.id;
            egzamin->lista[idx].zdana_matura = buf.zdana_matura;
            egzamin->lista[idx].zdana_teoria_wczesniej = buf.zdana_teoria_wczesniej;
            
            egzamin->lista[idx].zaliczona_A = 0;
            egzamin->lista[idx].zaliczona_B = 0;
            egzamin->lista[idx].punkty_teoria = 0;
            egzamin->lista[idx].punkty_praktyka = 0;

            if (buf.zdana_matura == 1) {
                if (buf.zdana_teoria_wczesniej == 1) {
                    int ocena = (rand() % 71) + 30;
                    egzamin->lista[idx].punkty_teoria = ocena; 
                    
                    egzamin->lista[idx].status = 1; 
                    sem_post(sem_kolejka_A); 
                    
                    dodaj_do_loggera(plik_logu, "[Dziekan] [PID: %d] Kandydat [PID: %d] skierowany do komisji A [PID: %d] |\t Kandydat zdał teorię wcześniej\n", 
                           (int)getpid(), buf.id, komisje_pids[0]);
                } else {
                    egzamin->lista[idx].status = 1; 
                    sem_post(sem_kolejka_A);
                    
                    dodaj_do_loggera(plik_logu, "[Dziekan] [PID: %d] Kandydat [PID: %d] skierowany do komisji A [PID: %d]\n", 
                           (int)getpid(), buf.id, komisje_pids[0]);
                }
            } else {
                egzamin->lista[idx].status = 3; 
                dodaj_do_loggera(plik_logu, "[Dziekan] [PID: %d] Kandydat [PID: %d] zarejestrowany w systemie, ale odrzucony (Brak Matury).\n", (int)getpid(), buf.id);
            }
            egzamin->liczba_kandydatow++; 
        }
    }
    
    close(fd_fifo);
    unlink(FIFO_WEJSCIE);
    
    dodaj_do_loggera(plik_logu, "[Dziekan] [PID: %d] Zakończono wpuszczanie. Czekam aż wszyscy studenci wyjdą...\n", (int)getpid());

    for (int i = 0; i < LICZBA_KANDYDATOW; i++) {
        wait(NULL); 
    }
    
    dodaj_do_loggera(plik_logu, "[Dziekan] [PID: %d] Wszyscy kandydaci opuścili system. Zamykam komisje.\n", (int)getpid());
    for (int i = 0; i < LICZBA_KOMISJI; i++) {
        kill(komisje_pids[i], SIGTERM);
    }
    
    wait(NULL); 
    wait(NULL);

    dodaj_do_loggera(plik_logu, "\n[Dziekan] [PID: %d] Generuję listy rankingowe...\n", (int)getpid());
    
    int liczba_uczestnikow = egzamin->liczba_kandydatow;
    Student *kopia_studentow = malloc(sizeof(Student) * liczba_uczestnikow);
    
    for (int i = 0; i < liczba_uczestnikow; i++) {
        kopia_studentow[i] = egzamin->lista[i];
    }

    for (int i = 0; i < liczba_uczestnikow - 1; i++) {
        for (int j = 0; j < liczba_uczestnikow - i - 1; j++) {
            double suma1 = kopia_studentow[j].punkty_teoria + kopia_studentow[j].punkty_praktyka;
            double suma2 = kopia_studentow[j+1].punkty_teoria + kopia_studentow[j+1].punkty_praktyka;

            if (suma1 < suma2) {
                Student temp = kopia_studentow[j];
                kopia_studentow[j] = kopia_studentow[j+1];
                kopia_studentow[j+1] = temp;
            }
        }
    }

    FILE *plik_ranking = fopen("lista_rankingowa.txt", "w");
    if (plik_ranking) {
        fprintf(plik_ranking, "PID | OCENA A | OCENA B | OGOLNA OCENA | STATUS\n");
        
        int licznik_zakwalifikowanych = 0; 

        for (int i = 0; i < liczba_uczestnikow; i++) {
            Student s = kopia_studentow[i];
            double ogolna = (s.punkty_teoria + s.punkty_praktyka) / 2.0;
            
            char *status = "OBLAL";

            if (s.zdana_matura == 0) {
                status = "BRAK_MATURY";
            }
            else if (s.zaliczona_A == 1 && s.zaliczona_B == 1) {
                if (licznik_zakwalifikowanych < LIMIT_PRZYJEC) {
                    status = "ZDAL_EGZAMIN_PRZYJETY";
                    licznik_zakwalifikowanych++; 
                } else {
                    status = "ZDAL_EGZAMIN_BRAK_MIEJSC"; 
                }
            }
            
            fprintf(plik_ranking, "%d | %.2lf | %.2lf | %.2lf | %s\n", 
                    s.id, s.punkty_teoria, s.punkty_praktyka, ogolna, status);
        }
        fclose(plik_ranking);
    }

    FILE *plik_przyjec = fopen("lista_przyjetych.txt", "w");
    if (plik_przyjec) {
        fprintf(plik_przyjec, "PID | OCENA A | OCENA B | OGOLNA OCENA\n");
        
        int licznik_przyjetych = 0;
        
        for (int i = 0; i < liczba_uczestnikow; i++) {
            Student s = kopia_studentow[i];
            
            if (s.zaliczona_A == 1 && s.zaliczona_B == 1) {
                if (licznik_przyjetych < LIMIT_PRZYJEC) {
                    double ogolna = (s.punkty_teoria + s.punkty_praktyka) / 2.0;
                    fprintf(plik_przyjec, "%d | %.2lf | %.2lf | %.2lf\n", 
                            s.id, s.punkty_teoria, s.punkty_praktyka, ogolna);
                    licznik_przyjetych++;
                }
            }
        }
        fclose(plik_przyjec);
    }

    free(kopia_studentow);

    dodaj_do_loggera(plik_logu, "[Dziekan] [PID: %d] Raporty gotowe (Limit przyjęć: %d).\n", (int)getpid(), LIMIT_PRZYJEC);
    
    if (plik_logu) fclose(plik_logu);
    shm_unlink(SHM_NAME); 
    return 0;
}