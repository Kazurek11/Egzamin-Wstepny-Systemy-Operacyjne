# Egzamin Wstępny - dokumentacja projektu Kacper Figura

## 1.Polecenie - problem do rozwiazania

Na pewnej uczelni zorganizowano egzamin wstępny na kierunek informatyka. Liczba miejsc wynosi
M (np. M=120), liczba chętnych ok. 10 osób na jedno miejsce. Kandydaci gromadzą się przed
budynkiem wydziału czekając w kolejce na wejście. Warunkiem udziału w egzaminie jest zdana
matura (ok. 2% kandydatów nie spełnia tego warunku). O określonej godzinie T dziekan wpuszcza
kandydatów na egzamin, sprawdzając jednocześnie, czy dana osoba ma zdaną maturę – w tym
momencie dziekan tworzy listę kandydatów i listę osób niedopuszczonych do egzaminu (id procesu).
Egzamin składa się z 2 części: części teoretycznej (komisja A) i części praktycznej (komisja B).
Komisja A składa się z 5 osób, komisja B składa się z 3 osób. Komisje przyjmują kandydatów w
osobnych salach.
20
Każda z osób w komisji zadaje po jednym pytaniu, pytania są przygotowywane na bieżąco (co losową
liczbę sekund) w trakcie egzaminu. Może zdarzyć się sytuacja w której, członek komisji spóźnia się z
zadaniem pytania wówczas kandydat czeka aż otrzyma wszystkie pytania. Po otrzymaniu pytań
kandydat ma określony czas Ti na przygotowanie się do odpowiedzi. Po tym czasie kandydat udziela
komisji odpowiedzi (jeżeli w tym czasie inny kandydat siedzi przed komisją, musi zaczekać aż zwolni
się miejsce), które są oceniane przez osobę w komisji, która zadała dane pytanie (ocena za każdą
odpowiedź jest losowana - wynik procentowy w zakresie 0-100%). Przewodniczący komisji (jedna z
osób) ustala ocenę końcową z danej części egzaminu (wynik procentowy w zakresie 0-100%).
Do komisji A kandydaci wchodzą wg listy otrzymanej od dziekana. Do danej komisji może wejść
jednocześnie maksymalnie 3 osoby.
Zasady przeprowadzania egzaminu:
• Kandydaci w pierwszej kolejności zdają egzamin teoretyczny.
• Jeżeli kandydat zdał część teoretyczną na mniej niż 30% nie podchodzi do części
praktycznej.
• Po pozytywnym zaliczeniu części teoretycznej (wynik >30%) kandydat staje w kolejce do
komisji B.
• Wśród kandydatów znajdują się osoby powtarzające egzamin, które mają już zaliczoną część
teoretyczną egzaminu (ok. 2% kandydatów) – takie osoby informują komisję A, że mają
zdaną część teoretyczną i zdają tylko część praktyczną.
• Listę rankingową z egzaminu tworzy Dziekan po pozytywnym zaliczeniu obu części egzaminu
– dane do Dziekana przesyłają przewodniczący komisji A i B.
• Po wyjściu ostatniego kandydata Dziekan publikuje listę rankingową oraz listę przyjętych. Na
listach znajduje się id kandydata z otrzymanymi ocenami w komisji A i B oraz oceną końcową
z egzaminu.
Na komunikat (sygnał1) o ewakuacji – sygnał wysyła Dziekan - kandydaci natychmiast przerywają
egzamin i opuszczają budynek wydziału – Dziekan publikuje listę kandydatów wraz z ocenami, którzy
wzięli udział w egzaminie wstępnym.
Napisz programy Dziekan, Komisja i Kandydat symulujące przeprowadzenie egzaminu wstępnego.
Raport z przebiegu symulacji zapisać w pliku (plikach) tekstowym.

## Problematyka

---

## 4. Przebieg inicjalizacji i struktura procesów symulacji

Symulacja realizowana jest w architekturze wieloprocesowej. Centralną jednostką zarządzającą jest proces **Dziekana**, który powołuje do życia infrastrukturę IPC (Inter-Process Communication) oraz procesy potomne: **Komisje** i **Kandydatów**. Poniższy opis przedstawia chronologiczny przebieg procedur startowych.

### 4.1. Inicjalizacja środowiska w procesie Dziekana

Proces Dziekana (uruchamiany przez `Makefile`) rozpoczyna działanie od przygotowania stabilnego środowiska dla symulacji.

**1. Obsługa sygnałów i logowanie**
W pierwszej kolejności konfigurowany jest system logowania oraz rejestrowane są handlery sygnałów systemowych:

* `SIGCHLD`: Obsługiwany przez `handler_smierci_dziecka`. Procedura ta monitoruje nieoczekiwane zakończenie procesów potomnych, zapobiegając powstawaniu tzw. procesów zombie oraz reagując na krytyczne awarie (np. zabicie procesu komisji), co skutkuje kontrolowanym zakończeniem całej symulacji.
* `SIGINT` / `SIGUSR1`: Obsługiwane przez `handler_ewakuacji`. Umożliwiają one bezpieczne przerwanie symulacji przez użytkownika i wygenerowanie raportów cząstkowych przed zamknięciem programu.

**2. Czyszczenie i tworzenie kanałów komunikacyjnych**
Następnie wykonywane jest czyszczenie zasobów systemowych (`unlink`, `shm_unlink`, `sem_unlink`), aby usunąć pozostałości po poprzednich uruchomieniach. Dopiero na tak przygotowanym gruncie tworzona jest nazwana kolejka **FIFO (potok)** z uprawnieniami `0600`. Służy ona do jednokierunkowego przesyłania struktur zgłoszeniowych od Kandydatów do Dziekana.

**3. Alokacja Pamięci Dzielonej i Synchronizacja**
Tworzony jest obiekt pamięci dzielonej (`shm_open`, `mmap`), który będzie przechowywał główną strukturę `EgzaminPamiecDzielona` z tablicą studentów. Aby umożliwić bezpieczny dostęp do tej pamięci z poziomu wielu procesów, następuje **inicjalizacja mechanizmów synchronizacji**:

* **Atrybuty Międzyprocesowe:** Tworzone są atrybuty mutexów (`pthread_mutexattr_t`) i zmiennych warunkowych (`pthread_condattr_t`) z flagą **`PTHREAD_PROCESS_SHARED`**. Jest to niezbędne, aby obiekty te działały między odrębnymi procesami, a nie tylko wątkami.
* **Synchronizacja Globalna:** Inicjowany jest `mutex_rejestracji` (chroniący tablicę podczas zapisu Dziekana) oraz `cond_rejestracji` (służąca do usypiania kandydatów oczekujących na wpis).
* **Synchronizacja Drobnoziarnista (Per-Student):** W pętli iterującej od 0 do `MAX_KANDYDATOW` inicjowane są indywidualne mechanizmy dla każdego slotu studenta:
* `mutex_ipc`: Blokuje dostęp tylko do rekordu konkretnego studenta. Dzięki temu zapisywanie danych jednego kandydata nie blokuje dostępu do reszty pamięci, co maksymalizuje współbieżność.
* `cond_ipc`: Służy do precyzyjnej sygnalizacji między Komisją a konkretnym Kandydatem.

* **Czyszczenie:** Po zakończeniu inicjalizacji, atrybuty (`mattr`, `cattr`) są niszczone (`destroy`), gdyż posłużyły jedynie jako wzorzec konfiguracyjny. Same mutexy pozostają aktywne w pamięci.

### 4.2. Inicjalizacja Semaforów Nazwanych

Kluczowym elementem sterowania przepływem są semafory nazwane POSIX. Są one tworzone w procesie Dziekana z flagą `O_CREAT` i uprawnieniami **`0600`** (odczyt/zapis tylko dla właściciela procesu), co zapewnia izolację symulacji od innych użytkowników systemu. Dziekan weryfikuje poprawność utworzenia każdego semafora, a następnie zamyka uchwyty (`sem_close`) do tych, których sam nie używa, pozostawiając je w systemie dla procesów potomnych.

Utworzone semafory:

* `KOLEJKA_KOMISJA_A` (start: 0): Podnoszony przez Dziekana, opuszczany przez Komisję A. Sygnalizuje gotowość kandydata.
* `KOLEJKA_KOMISJA_B` (start: 0): Podnoszony przez Komisję A, opuszczany przez Komisję B.
* `WOLNE_MIEJSCA_KOMISJA_A / B` (start: 3): Semafory zliczające, reprezentujące fizyczne miejsca (krzesła) w salach.
* `SEM_SYNC_START` (start: 0): Służy do synchronizacji procesu `fork()` (opis w sekcji 4.4).
* `SEM_LICZNIK_KONCA` (start: 0): Zlicza zakończone procesy kandydatów, pozwalając Dziekanowi określić moment zakończenia symulacji.

### 4.3. Tworzenie Procesów Komisji

Jeszcze przed pojawieniem się kandydatów, Dziekan tworzy dwa procesy potomne realizujące kod `./komisja`.

1. W pętli wywoływany jest `fork()`.
2. Proces potomny nadpisuje swój kod funkcją `execl`, przyjmując argument typu komisji ("A" lub "B").
3. Dziekan zapisuje PID-y komisji w tablicy globalnej, aby w razie awarii móc wysłać do nich sygnał `SIGKILL`.

### 4.4. Tworzenie i Logika Procesów Kandydatów

Proces generowania kandydatów odbywa się w pętli. Aby uniknąć przeciążenia systemu (tzw. fork bomb) i błędów `EAGAIN`, zastosowano mechanizm ścisłej synchronizacji przy użyciu semafora `SEM_SYNC_START`.

**Mechanizm synchronizacji startu:**

1. **Dziekan:** Wykonuje `fork()`. Jeśli się uda, natychmiast wykonuje `sem_wait(sem_sync)`, blokując swoje działanie.
2. **Kandydat:** Uruchamia się (`execl`), wykonuje wstępną konfigurację i podnosi semafor `sem_post(sem_sync)`.
3. **Dziekan:** Dopiero po otrzymaniu sygnału od dziecka odblokowuje się, inkrementuje licznik procesów i przechodzi do tworzenia kolejnego kandydata.

**Logika działania Kandydata (Startup):**
Po uruchomieniu kandydat wykonuje następujące kroki:

1. **Losowanie atrybutów:** Zgodnie z logiką (zdana teoria -> zdana matura), najpierw losowane jest 2% kandydatów ze zdaną teorią. Z pozostałej puli losowane jest 2% pechowców bez zdanej matury. Reszta otrzymuje zdaną maturę bez teorii.
2. **Wysłanie zgłoszenia:** Kandydat wysyła strukturę `Zgloszenie` przez kolejkę FIFO do Dziekana.
3. **Selekcja wstępna:** Jeśli wylosowano brak matury, proces kończy działanie zaraz po wysłaniu zgłoszenia a Dziekan odnotowuje jego wykluczenie z dalszej rekturacji -> obniża semafor `SEM_LICZNIK_KONCA` (opis w sekcji 4.5).
4. **Dostęp do Pamięci Dzielonej:** Kandydaci z maturą mapują pamięć (`mmap`). W przypadku błędu mapowania proces kończy działanie.
5. **Oczekiwanie na rejestrację:** Kandydat wchodzi w sekcję krytyczną (`mutex_rejestracji`). Sprawdza, czy został wpisany na listę. Jeśli nie, zasypia na zmiennej warunkowej `cond_rejestracji`, czekając na sygnał `broadcast` od Dziekana.

### 4.5. Wewnętrzna Inicjalizacja Procesów Komisji

Równolegle do działań Dziekana, procesy Komisji A i B wykonują własną inicjalizację zasobów.

**1. Zasoby IPC i Pamięć**
Każda komisja mapuje pamięć dzieloną oraz otwiera odpowiednie semafory nazwane:

* Komisja A: `KOLEJKA_KOMISJA_A` (wejście), `KOLEJKA_KOMISJA_B` (wyjście), `WOLNE_MIEJSCA_A`.
* Komisja B: `KOLEJKA_KOMISJA_B` (wejście), `WOLNE_MIEJSCA_B`.
Obie komisje otwierają też `SEM_LICZNIK_KONCA` do raportowania przeanalizowanych studentów. Komisja A może jedynie podnieść licznik w przypadku odrzucenia a komisja B podnosi licznik nie zależnie od wyniku egzaminu.

**2. Konfiguracja Stanowisk i Barier**
Komisja przygotowuje tablicę stanowisk (stolików). Dla każdego stanowiska inicjalizowane są:

* `mutex`: Chroni dane stolika podczas egzaminu.
* **Bariera (`pthread_barrier_t`)**: Jest to mechanizm synchronizacji grupowej, inicjowany na liczbę  członków komisji ( dla A,  dla B).
* **Cel działania bariery:** Bariera działa jak punkt zbiórki ("szlaban"). Zatrzymuje ona wątki członków komisji w jednym punkcie kodu do momentu, aż zamelduje się tam wymagana liczba egzaminatorów. Gwarantuje to, że egzamin przy stoliku rozpocznie się (lub zakończy) dopiero wtedy, gdy **cały kompletny skład komisji** jest obecny.

**3. Tworzenie Wątków**
Na końcu tworzone są wątki realizujące logikę egzaminatorów:

* Wątek o `id=0`: Realizuje funkcję **Przewodniczącego** (pobiera studentów z semafora kolejki, przydziela stoliki).
* Wątki o `id>0`: Realizują funkcję **Członków Komisji** (przeprowadzają egzamin przy stolikach, synchronizując się na barierach).
* Liczba wątków jest dostosowana do typu komisji (1+4 dla A, 1+2 dla B).
  (...)
4.    Watki istnieja tak długo az dziekan ich nie poinformuje o zakończeniu wpuszczania kandydatów
5.     for (int i = 0; i < liczba_czlonkow; i++) {
        pthread_join(egzaminatorzy[i], NULL);
    }

### 4.6 Logika działania komisji

#### 4.6.1 Logika działania przwodniczącego komisji

    int P_id = *(int*)arg;
    free(arg); 
    dodaj_do_loggera(plik_logu, "[Komisja A] [P] Przewodniczący %d w komisji A: Rozpoczynam pracę.\n", P_id);

    while (1) {
        sem_wait(sem_kolejka_komisji); 
        sem_wait(sem_miejsca_komisji); 

        int stolik = znajdz_wolne_stanowisko();
        
        pthread_mutex_lock(&stanowiska[stolik].mutex);
        int id_kandydata_PD = -1;
        int kandydat_pid = -1;


    Z poczatku przewodniczacy komisji przypisuje swoje id z pamięci zaalokowanej w systemie operacyjnym. Nastepnie zwalnia miejsce w pamieci i informuje o rozpoczaciu pracy.
    Obniża semafor kolejki komisji (jezeli semafor > 0) Co gwarantuje nam nie pominiecie zadnego z kandydatów oraz to że komisja nie zacznie swojej pracy bez kandydata, ponieważ semafor ten domyślnie ustawiony jest na wartość 0. W drugiej kolejności obniża semafor miejsca w komisji czyli wolnego stolika. Jeżeli takiego miejsca w danym momencie nie ma to czeka aż miejsce się zwolni. Następnie realizuje funkcje `znajdz_wolne_stanowisko` (pełen opis w punkcie X) która zwraca numer indetyfikacyjny stanowiska w pamięci (od 0-2 poniewaz MAX MIEJSC == 3) kiedy znajdzie wolne stanowisko to odrazu rezerwuje stolik i przechodzi do szuakania kandydata na to miejsce

    
        for (int i = 0; i < egzamin->liczba_kandydatow; i++) {
            if (egzamin->lista[i].status == 1) { 
                egzamin->lista[i].status = 11;  
                id_kandydata_PD = i;
                kandydat_pid = egzamin->lista[i].id;
                break;
            }
        }

    szuka kandydata i zmienia jego status na 11 co oznacza ze kandydat znajduje sie trakcie egzaminu. 

    
        if (id_kandydata_PD == -1) {
            stanowiska[stolik].zajete = 0;
            pthread_mutex_unlock(&stanowiska[stolik].mutex);
            sem_post(sem_miejsca_komisji);
        
            sem_post(sem_kolejka_komisji);
            continue; 
        }

    Nastepnie w razie nie znalezienia kandydata 

## Opis procedur komisji

## Podumowanie
