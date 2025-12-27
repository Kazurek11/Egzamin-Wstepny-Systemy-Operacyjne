#include "common.h"
#include <sys/wait.h>
#include <errno.h>

int main() {
    if (unlink(FIFO_WEJSCIE) == -1) {
        if (errno != ENOENT) { // Ignorujemy błąd, jeśli plik po prostu nie istnieje
            perror("[Dziekan] Błąd unlink (sprzątanie)");
        }
    }

    // 2. Tworzenie nowej kolejki
    if (mkfifo(FIFO_WEJSCIE, 0600) == -1) {
        perror("[Dziekan] Błąd mkfifo");
        return 3;
    }

    printf("[Dziekan] Rekrutacja otwarta. Generuję kandydatów...\n");

    // 3. Generowanie procesów
    for (int i = 0; i < LICZBA_KANDYDATOW; i++) {
        pid_t pid = fork();
        
        if (pid == -1) {
            perror("[Dziekan] Błąd fork");
            // W razie błędu forka, można spróbować kontynuować z mniejszą liczbą,
            // albo przerwać. Tutaj przerywamy dla bezpieczeństwa.
            unlink(FIFO_WEJSCIE); 
            return 2; 
        }

        if (pid == 0) {
            // --- PROCES DZIECKO (Kandydat) ---
            execl("./kandydat", "kandydat", NULL);
            
            perror("[Dziekan -> Child] Błąd execl");
            return 1;
        }
        
        // --- PROCES RODZIC (Dziekan) ---
        // opóźnienie, żeby tworzenie procesów było widoczne w czasie
        usleep(5000); // 5ms
    }

    printf("[Dziekan] Wszyscy kandydaci (%d) stoją przed wydziałem.\n", LICZBA_KANDYDATOW);
    printf("[Dziekan] Czekam na godzinę T (symulacja 2 sekundy)...\n");
    sleep(2); // Czas, w którym kandydaci wiszą na open - do usuniecia

    printf("[Dziekan] Godzina T wybija! Otwieram drzwi (FIFO).\n");
    
    // Otwarcie w trybie READ ONLY odblokuje kandydatów czekających na WRITE ONLY
    int fd = open(FIFO_WEJSCIE, O_RDONLY);
    if (fd == -1) {
        perror("[Dziekan] Błąd open FIFO");
        unlink(FIFO_WEJSCIE);
        return 3;
    }

    // 5. Odbieranie zgłoszeń po stronie dziekana
    Zgloszenie zgloszenie_dziekan;
    int licznik_ogolny = 0;
    int licznik_zdana_matura = 0;
    while (read(fd, &zgloszenie_dziekan, sizeof(Zgloszenie)) > 0) {
        // Czytamy dopóki są kandydaci. 
        // Gdy ostatni kandydat zamknie swoje fd, read zwróci 0 (EOF).
        licznik_ogolny++;
        // logika matury
        if (zgloszenie_dziekan.zdana_matura == 1){
            licznik_zdana_matura++;
            // tworzenie listy kandydatów 
        }
        printf("[Dziekan] Odebrano zgłoszenie od PID: %d, Matura: %d\n", zgloszenie_dziekan.id, zgloszenie_dziekan.zdana_matura);
    }

    printf("[Dziekan] Zakończono przyjmowanie.\n Obsłużono: %d osób.\n", licznik_ogolny);
    printf("[Dziekan] Na liste kandydatów zapisano %d osób.\n", licznik_zdana_matura);
    printf("[Dziekan] Na liste osób niedopuszonych do egzaminu zapisano: %d osób.\n", (licznik_ogolny - licznik_zdana_matura));

    close(fd);
    unlink(FIFO_WEJSCIE);

    // Czekamy na wszystkie dzieci, żeby nie zostały zombie
    while (wait(NULL) > 0); 
    
    return 0;
}