#include "common.h"

int main() {
    srand(getpid()); 

    
    Zgloszenie zgloszenie_kandydat;
    zgloszenie_kandydat.id = getpid();
    if ((rand() % 100) < SZANSA_NA_ZDANA_TEORIE) {
        // osoba powtarzajaca
        zgloszenie_kandydat.zdana_matura = 1;       // MUSI mieć zdaną maturę
        zgloszenie_kandydat.zdana_teoria_wczesniej = 1; // Ma już zaliczoną teorię z poprzednich lat
    } 
    else {
        // nowy kandydat
        zgloszenie_kandydat.zdana_teoria_wczesniej = 0; 

        // losowanie matury dla nowego kandydata
        if ((rand() % 100) < SZANSA_NA_BRAK_MATURY) {
            zgloszenie_kandydat.zdana_matura = 0; 
        } else {
            zgloszenie_kandydat.zdana_matura = 1;
        }
    }

    // 1. Próba wejścia do kolejki (kandydat ma czekac na dziekana)
    int file_descriptor = open(FIFO_WEJSCIE, O_WRONLY);
    if (file_descriptor == -1) {
        perror("[Kandydat] Nie mogę wejść do kolejki");
        return 1;
    }

    // 2. Wysłanie zgłoszenia
    // Sprawdzamy, czy udało się zapisać tyle bajtów, ile ma struktura
    ssize_t written_bytes = write(file_descriptor, &zgloszenie_kandydat, sizeof(Zgloszenie));
    
    if (written_bytes == -1) {
        perror("[Kandydat] Błąd wysyłania zgłoszenia");
        close(file_descriptor);
        return 2;
    }
    // 3. Sprawdzenia czy wysłano wszystkie bajty
    if (written_bytes != sizeof(Zgloszenie)) {
        fprintf(stderr, "[Kandydat] Błąd częściowego zapisu! Wysłano %ld z %ld bajtów.\n", written_bytes, sizeof(Zgloszenie));
        close(file_descriptor);
        return 3;
    }

    close(file_descriptor);
    return 0; 
}