#define _GNU_SOURCE
#include "common.h"

int main() {
    srand(time(NULL) ^ getpid());

    sem_unlink(KOLEJKA_KOMISJA_A);
    sem_unlink(KOLEJKA_KOMISJA_B);
    sem_unlink(WOLNE_MIEJSCA_KOMISJA_A);
    sem_unlink(WOLNE_MIEJSCA_KOMISJA_B);

    if (unlink(FIFO_WEJSCIE) == -1) {
        if (errno != ENOENT) {
            perror("[Dziekan] Błąd unlink");
        }
    }
    mkfifo(FIFO_WEJSCIE, 0600);
    shm_unlink(SHM_NAME);

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
    if (shm_fd == -1) {
        return 4;
    }
    ftruncate(shm_fd, sizeof(EgzaminPamiecDzielona));
    
    EgzaminPamiecDzielona *egzamin = mmap(NULL, sizeof(EgzaminPamiecDzielona), 
                                          PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (egzamin == MAP_FAILED) {
        perror("[Dziekan] Błąd mmap");
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
    
    if (sem_kolejka_A == SEM_FAILED || sem_kolejka_B == SEM_FAILED) {
        perror("[Dziekan] Błąd semaforow kolejkowych");
        return 6;
    }
    if (sem_miejsce_A == SEM_FAILED || sem_miejsce_B == SEM_FAILED) {
        perror("[Dziekan] Błąd semaforow wolnych miejsc");
        return 6;
    }
    
    sem_close(sem_miejsce_A);
    sem_close(sem_miejsce_B);

    egzamin->liczba_kandydatow = 0;

    printf("[Dziekan] [PID: %d] Rekrutacja otwarta. Generuję komisje i kandydatów \n", (int)getpid());

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

    for (int i = 0; i < LICZBA_KANDYDATOW; i++) {
        if (fork() == 0) {
            execl("./kandydat", "kandydat", NULL);
            return 6;
        }
        sleep_ms(5); 
    }

    printf("[Dziekan] [PID: %d] Wszyscy kandydaci (%d) czekają.\n", (int)getpid(), LICZBA_KANDYDATOW);
    printf("[Dziekan] [PID: %d] Czekam na godzinę T %d...\n", (int)getpid(), GODZINA_T);
    sleep(GODZINA_T); 
    printf("[Dziekan] [PID: %d] Godzina T wybija! Otwieram drzwi (FIFO).\n", (int)getpid());
    
    int fd = open(FIFO_WEJSCIE, O_RDONLY);
    
    FILE *plik_rekrutacja = fopen("lista_rekrutacja.txt", "w");
    FILE *plik_rekrutacja_odrzuceni = fopen("lista_rekrutacja_odrzuceni.txt", "w");

    if (plik_rekrutacja) {
        fprintf(plik_rekrutacja, "PID\tStatus Teorii\n");
    }
    if (plik_rekrutacja_odrzuceni) {
        fprintf(plik_rekrutacja_odrzuceni, "PID (Brak Matury)\n");
    }

    Zgloszenie buf;
    
    while (read(fd, &buf, sizeof(Zgloszenie)) > 0) {
        if (buf.zdana_matura == 1) {
            if (plik_rekrutacja) {
                fprintf(plik_rekrutacja, "%d\t%s\n", buf.id, buf.zdana_teoria_wczesniej ? "Zaliczona" : "Do zdania");
            }

            int idx = egzamin->liczba_kandydatow;
            if (idx < MAX_KANDYDATOW) {
                egzamin->lista[idx].id = buf.id;
                egzamin->lista[idx].zdana_matura = 1;
                egzamin->lista[idx].zdana_teoria_wczesniej = buf.zdana_teoria_wczesniej;
                
                egzamin->lista[idx].zaliczona_A = 0;
                egzamin->lista[idx].zaliczona_B = 0;
                egzamin->lista[idx].punkty_teoria = 0;
                egzamin->lista[idx].punkty_praktyka = 0;

                if (buf.zdana_teoria_wczesniej == 1) {
                    int ocena = (rand() % 71) + 30;
                    egzamin->lista[idx].punkty_teoria = ocena; 
                    
                    egzamin->lista[idx].status = 1; 
                    sem_post(sem_kolejka_A); 
                    
                    printf("[Dziekan] [PID: %d] Kandydat [PID: %d] skierowany do komisji A [PID: %d] |\t Kandydat zdał teorię wcześniej\n", 
                           (int)getpid(), buf.id, komisje_pids[0]);
                } else {
                    egzamin->lista[idx].status = 1; 
                    sem_post(sem_kolejka_A);
                    
                    printf("[Dziekan] [PID: %d] Kandydat [PID: %d] skierowany do komisji A [PID: %d]\n", 
                           (int)getpid(), buf.id, komisje_pids[0]);
                }
                egzamin->liczba_kandydatow++; 
            }
        } else {
            if (plik_rekrutacja_odrzuceni) {
                fprintf(plik_rekrutacja_odrzuceni, "%d\n", buf.id);
            }
            printf("[Dziekan] [PID: %d] Kandydat [PID: %d] odrzucony z powodu braku matury.\n", (int)getpid(), buf.id);
        }
    }

    if (plik_rekrutacja) {
        fclose(plik_rekrutacja);
    }
    if (plik_rekrutacja_odrzuceni) {
        fclose(plik_rekrutacja_odrzuceni);
    }
    
    close(fd);
    unlink(FIFO_WEJSCIE);
    
    printf("[Dziekan] [PID: %d] Zakończono wpuszczanie. Czekam aż wszyscy studenci wyjdą...\n", (int)getpid());

    for (int i = 0; i < LICZBA_KANDYDATOW; i++) {
        wait(NULL); 
    }
    
    printf("[Dziekan] [PID: %d] Wszyscy kandydaci opuścili system. Zamykam komisje.\n", (int)getpid());
    for (int i = 0; i < LICZBA_KOMISJI; i++) {
        kill(komisje_pids[i], SIGTERM);
    }
    
    wait(NULL); 
    wait(NULL);

    printf("\n[Dziekan] [PID: %d] Generuję listy rankingowe...\n", (int)getpid());
    
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

            if (s.zaliczona_A == 1 && s.zaliczona_B == 1) {
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

    printf("[Dziekan] [PID: %d] Raporty gotowe (Limit przyjęć: %d).\n", (int)getpid(), LIMIT_PRZYJEC);
    
    shm_unlink(SHM_NAME); 
    return 0;
}