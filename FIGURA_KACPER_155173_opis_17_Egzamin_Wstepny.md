# **EGZAMIN WSTĘPNY - dokumentacja projektu**

| Nazwisko Imię | Kierunek/Tryb            | Rok | Semestr | Temat Projektu            |
|:--------------|:-------------------------|:---:|:--------|:--------------------------|
| Kacper Figura | Informatyka/Stacjonarnie | II  | Zimowy  | Temat 17 – Egzamin wstępny|

## Spis treści dokumentacji

 **[0. Polecenie - problem do rozwiązania](#0-polecenie---problem-do-rozwiązania)**

 **[1. Wymagania i Instrukcja Uruchomienia](#1-wymagania-i-instrukcja-uruchomienia)**

  * [1.1. Wymagania systemowe](#11-wymagania-systemowe)
  * [1.2. Kompilacja i uruchomienie](#12-kompilacja-i-uruchomienie)
  * [1.3. Logi i wyniki](#13-logi-i-wyniki)

**[2. Przebieg inicjalizacji oraz struktura symulacji](#2-przebieg-inicjalizacji-oraz-struktura-symulacji)**

* [2.1. Inicjalizacja środowiska w procesie Dziekana](#21-inicjalizacja-środowiska-w-procesie-dziekana)
* [2.2. Inicjalizacja Semaforów Nazwanych](#22-inicjalizacja-semaforów-nazwanych)
* [2.3. Tworzenie Procesów Komisji](#23-tworzenie-procesów-komisji)
* [2.4. Tworzenie i Logika Procesów Kandydatów](#24-tworzenie-i-logika-procesów-kandydatów)
* [2.5. Wewnętrzna Inicjalizacja Procesów Komisji](#25-wewnętrzna-inicjalizacja-procesów-komisji)
* [2.6. Logika operacyjna Komisji](#26-logika-operacyjna-komisji)
  * [2.6.1. Algorytm Przewodniczącego Komisji I](#261-algorytm-przewodniczącego-komisji-i)
  * [2.6.2. Algorytm Członka Komisji I](#262-algorytm-członka-komisji-i)
  * [2.6.3. Procedura Egzaminacyjna](#263-procedura-egzaminacyjna)
  * [2.6.4. Koordynacja Egzaminu przez Przewodniczącego](#264-koordynacja-egzaminu-przez-przewodniczącego)
  * [2.6.5. Kandydat odpowiada komisji](#265-kandydat-odpowiada-komisji)
  * [2.6.6. Analiza odpowiedzi i ocena końcowa](#266-analiza-odpowiedzi-i-ocena-końcowa)
* [2.7. Finał procesu symulacji (Dziekan)](#27-finał-procesu-symulacji-dziekan)

**[3. Dokumentacja techniczna funkcji w projekcie](#3-dokumentacja-techniczna-funkcji-w-projekcie)**

* [3.1. Dziekan](#31-dziekan)
* [3.2. Komisja](#32-komisja)
* [3.3. Moduł Narzędziowy (Common)](#33-moduł-narzędziowy-common)

**[4. Testy](#4-testy)**

* [4.1. Test scenariusza awarii: Nagłe zakończenie procesu kandydata (SIGKILL)](#41-test-scenariusza-awarii-nagłe-zakończenie-procesu-kandydata-sigkill)
* [4.2. Scenariusz testowy: Całkowity brak uprawnień (100% niezdanych matur)](#42-scenariusz-testowy-całkowity-brak-uprawnień-100-niezdanych-matur)
* [4.3. Scenariusz testowy: Zaliczenie teorii w latach ubiegłych](#43-scenariusz-testowy-zaliczenie-teorii-w-latach-ubiegłych)
* [4.4. Weryfikacja mechanizmu limitów przyjęć (Numerus Clausus)](#44-weryfikacja-mechanizmu-limitów-przyjęć-numerus-clausus)
* [4.5. Weryfikacja nadmiarowości miejsc](#45-weryfikacja-nadmiarowości-miejsc-limit-przyjęć--liczba-kandydatów)

**[5. Problemy napotkane podczas realizacji projektu](#5-problemy-napotkane-podczas-realizacji-projektu)**

* [5.1. Wyścig wątków](#51-wyścig-wątków)
* [5.2. Zakleszczenia przy synchronizacji](#52-zakleszczenia-przy-synchronizacji)
* [5.3. Procesy Zombie](#53-procesy-zombie)
* [5.4. Trwałość obiektów IPC po awarii](#54-trwałość-obiektów-ipc-po-awarii)
* [5.5. Synchronizacja grupowa przy stoliku](#55-synchronizacja-grupowa-przy-stoliku)
* [5.6. Analiza wykorzystywanych struktur](#56-analiza-wykorzystywanych-struktur-optymalizacja-synchronizacji)

**[6. Implementacja wymaganych konstrukcji systemowych](#6-implementacja-wymaganych-konstrukcji-systemowych-linki-do-github)**

* [6.1. Tworzenie i obsługa plików](#61-tworzenie-i-obsługa-plików)
* [6.2. Tworzenie procesów](#62-tworzenie-procesów)
* [6.3. Tworzenie i obsługa wątków](#63-tworzenie-i-obsługa-wątków)
* [6.4. Obsługa sygnałów](#64-obsługa-sygnałów)
* [6.5. Synchronizacja procesów (Semafory POSIX)](#65-synchronizacja-procesów-semafory-posix)
* [6.6. Łącza nazwane (FIFO)](#66-łącza-nazwane-fifo)
* [6.7. Segmenty pamięci dzielonej (POSIX SHM)](#67-segmenty-pamięci-dzielonej-posix-shm)

**[7. Podsumowanie](#7-podsumowanie)**

* [7.1. Wyzwania projektowe](#71-wyzwania-projektowe)
* [7.2. Optymalizacja struktury danych i zarządzania zasobami](#72-optymalizacja-struktury-danych-i-zarządzania-zasobami)
* [7.3. Wynik końcowy](#73-wynik-końcowy)

---

## 0. Polecenie - problem do rozwiązania

Na pewnej uczelni zorganizowano egzamin wstępny na kierunek informatyka. Liczba miejsc wynosi **M** (np. M=120), liczba chętnych ok. 10 osób na jedno miejsce. Kandydaci gromadzą się przed budynkiem wydziału czekając w kolejce na wejście. Warunkiem udziału w egzaminie jest zdana matura (ok. 2% kandydatów nie spełnia tego warunku).

O określonej godzinie **T** dziekan wpuszcza kandydatów na egzamin, sprawdzając jednocześnie, czy dana osoba ma zdaną maturę – w tym momencie dziekan tworzy listę kandydatów i listę osób niedopuszczonych do egzaminu (id procesu).

Egzamin składa się z 2 części: części teoretycznej (**komisja A**) i części praktycznej (**komisja B**). Komisja A składa się z 5 osób, komisja B składa się z 3 osób. Komisje przyjmują kandydatów w osobnych salach.

Każda z osób w komisji zadaje po jednym pytaniu, pytania są przygotowywane na bieżąco (co losową liczbę sekund) w trakcie egzaminu. Może zdarzyć się sytuacja w której, członek komisji spóźnia się z zadaniem pytania wówczas kandydat czeka aż otrzyma wszystkie pytania. Po otrzymaniu pytań kandydat ma określony czas **Ti** na przygotowanie się do odpowiedzi. Po tym czasie kandydat udziela komisji odpowiedzi (jeżeli w tym czasie inny kandydat siedzi przed komisją, musi zaczekać aż zwolni się miejsce), które są oceniane przez osobę w komisji, która zadała dane pytanie (ocena za każdą odpowiedź jest losowana - wynik procentowy w zakresie 0-100%). Przewodniczący komisji (jedna z osób) ustala ocenę końcową z danej części egzaminu (wynik procentowy w zakresie 0-100%).

Do komisji A kandydaci wchodzą wg listy otrzymanej od dziekana. Do danej komisji może wejść jednocześnie maksymalnie 3 osoby.

**Zasady przeprowadzania egzaminu:**

* Kandydaci w pierwszej kolejności zdają egzamin teoretyczny.
* Jeżeli kandydat zdał część teoretyczną na mniej niż 30% nie podchodzi do części praktycznej.
* Po pozytywnym zaliczeniu części teoretycznej (wynik >30%) kandydat staje w kolejce do komisji B.
* Wśród kandydatów znajdują się osoby powtarzające egzamin, które mają już zaliczoną część teoretyczną egzaminu (ok. 2% kandydatów) – takie osoby informują komisję A, że mają zdaną część teoretyczną i zdają tylko część praktyczną.
* Listę rankingową z egzaminu tworzy Dziekan po pozytywnym zaliczeniu obu części egzaminu – dane do Dziekana przesyłają przewodniczący komisji A i B.
* Po wyjściu ostatniego kandydata Dziekan publikuje listę rankingową oraz listę przyjętych. Na listach znajduje się id kandydata z otrzymanymi ocenami w komisji A i B oraz oceną końcową z egzaminu.

Na komunikat (sygnał1) o ewakuacji – sygnał wysyła Dziekan - kandydaci natychmiast przerywają egzamin i opuszczają budynek wydziału – Dziekan publikuje listę kandydatów wraz z ocenami, którzy wzięli udział w egzaminie wstępnym.

Napisz programy Dziekan, Komisja i Kandydat symulujące przeprowadzenie egzaminu wstępnego. Raport z przebiegu symulacji zapisać w pliku (plikach) tekstowym.

## 1. Wymagania i Instrukcja Uruchomienia

Projekt został zaimplementowany w języku C z wykorzystaniem standardu POSIX, co wymaga odpowiedniego środowiska systemowego do poprawnej obsługi mechanizmów IPC (Pamięć Dzielona, Semafory, Kolejki FIFO).

### 1.1. Wymagania systemowe

* **System operacyjny:** Linux (np. Ubuntu, Debian) lub Windows z podsystemem WSL (Windows Subsystem for Linux).
* *Uwaga:* Projekt korzysta z bibliotek `pthread` oraz `rt` (real-time), które są natywne dla systemów uniksowych. Uruchomienie bezpośrednio na Windows (bez WSL) lub macOS może wymagać dodatkowej konfiguracji.


* **Kompilator:** GCC (GNU Compiler Collection).
* **Narzędzia:** `make` (do automatyzacji procesu kompilacji).

### 1.2. Kompilacja i uruchomienie

W katalogu głównym projektu znajduje się plik `Makefile`, który automatyzuje proces budowania wszystkich modułów (`dziekan`, `komisja`, `kandydat`).

**Kroki do uruchomienia symulacji:**

1. **Czyszczenie środowiska (opcjonalne):**
Aby usunąć stare pliki binarne oraz logi z poprzednich uruchomień:

```bash
make clean

```

2. **Kompilacja projektu oraz uruchomienie:**

Aby skompilować wszystkie pliki źródłowe, zlinkować biblioteki (`-lpthread`, `-lrt`) i zarazem uruchomić symulacje:

```bash
make run
```

Poprawna kompilacja powinna utworzyć trzy pliki wykonywalne: `dziekan`, `komisja`, `kandydat`.
3. **Problemy z kompilacja**

W przypadku zapomnienia struktury kompilacji Makefile:

```bash
make help

```

4. **Usuniecie folderu log wraz z zawartościa**

W przypadku gdy chcemy usunać historyczne logi sumulacji:

```bash
make clean_logger

```

5. **Zakończenie działania:**

Program kończy się automatycznie po obsłużeniu wszystkich kandydatów. W przypadku konieczności nagłego przerwania symulacji (symulacja ewakuacji), należy użyć kombinacji klawiszy:
`Ctrl + C` (wysłanie sygnału `SIGINT`).

### 1.3. Logi i wyniki

Po zakończeniu działania programu, w folderze projektu pojawią się następujące pliki wynikowe:

* Katalog `/logi` – zawierający szczegółowe logi przebiegu symulacji dla każdego procesu (jeden loger dla kandydatów - łączy wszystkie procesy kandydatów).
* `lista_rankingowa.txt` – posortowana lista wszystkich uczestników wraz z ich statusami i wynikami.
* `lista_przyjetych.txt` – lista osób, które pomyślnie przeszły rekrutację z wynikiem punktowym.

## 2. Przebieg inicjalizacji oraz struktura symulacji

Symulacja realizowana jest w architekturze wieloprocesowej. Centralną jednostką zarządzającą jest proces **Dziekana**, który powołuje do życia infrastrukturę IPC (Inter-Process Communication) oraz procesy potomne: **Komisje** i **Kandydatów**. Poniższy opis przedstawia chronologiczny przebieg procedur startowych.

### 2.1. Inicjalizacja środowiska w procesie Dziekana

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

### 2.2. Inicjalizacja Semaforów Nazwanych

Kluczowym elementem sterowania przepływem są semafory nazwane POSIX. Są one tworzone w procesie Dziekana z flagą `O_CREAT` i uprawnieniami **`0600`** (odczyt/zapis tylko dla właściciela procesu), co zapewnia izolację symulacji od innych użytkowników systemu. Dziekan weryfikuje poprawność utworzenia każdego semafora, a następnie zamyka uchwyty (`sem_close`) do tych, których sam nie używa, pozostawiając je w systemie dla procesów potomnych.

Utworzone semafory:

* `KOLEJKA_KOMISJA_A` (start: 0): Podnoszony przez Dziekana, opuszczany przez Komisję A. Sygnalizuje gotowość kandydata.
* `KOLEJKA_KOMISJA_B` (start: 0): Podnoszony przez Komisję A, opuszczany przez Komisję B.
* `WOLNE_MIEJSCA_KOMISJA_A / B` (start: 3): Semafory zliczające, reprezentujące fizyczne miejsca (krzesła) w salach.
* `SEM_SYNC_START` (start: 0): Służy do synchronizacji procesu `fork()` (opis w sekcji 2.4).
* `SEM_LICZNIK_KONCA` (start: 0): Zlicza zakończone procesy kandydatów, pozwalając Dziekanowi określić moment zakończenia symulacji.

### 2.3. Tworzenie Procesów Komisji

Jeszcze przed pojawieniem się kandydatów, Dziekan tworzy dwa procesy potomne realizujące kod `./komisja`.

1. W pętli wywoływany jest `fork()`.
2. Proces potomny nadpisuje swój kod funkcją `execl`, przyjmując argument typu komisji ("A" lub "B").
3. Dziekan zapisuje PID-y komisji w tablicy globalnej, aby w razie awarii móc wysłać do nich sygnał `SIGKILL`.

### 2.4. Tworzenie i Logika Procesów Kandydatów

Proces generowania kandydatów odbywa się w pętli. Aby uniknąć przeciążenia systemu (tzw. fork bomb) i błędów `EAGAIN`, zastosowano mechanizm ścisłej synchronizacji przy użyciu semafora `SEM_SYNC_START`.

**Mechanizm synchronizacji startu:**

1. **Dziekan:** Wykonuje `fork()`. Jeśli się uda, natychmiast wykonuje `sem_wait(sem_sync)`, blokując swoje działanie.
2. **Kandydat:** Uruchamia się (`execl`), wykonuje wstępną konfigurację i podnosi semafor `sem_post(sem_sync)`.
3. **Dziekan:** Dopiero po otrzymaniu sygnału od dziecka odblokowuje się, inkrementuje licznik procesów i przechodzi do tworzenia kolejnego kandydata.

**Logika działania Kandydata (Startup):**
Po uruchomieniu kandydat wykonuje następujące kroki:

1. **Losowanie atrybutów:** Zgodnie z logiką (zdana teoria -> zdana matura), najpierw losowane jest 2% kandydatów ze zdaną teorią. Z pozostałej puli losowane jest 2% pechowców bez zdanej matury. Reszta otrzymuje zdaną maturę bez teorii.
2. **Wysłanie zgłoszenia:** Kandydat wysyła strukturę `Zgloszenie` przez kolejkę FIFO do Dziekana.
3. **Selekcja wstępna:** Jeśli wylosowano brak matury, proces kończy działanie zaraz po wysłaniu zgłoszenia a Dziekan odnotowuje jego wykluczenie z dalszej rekturacji -> obniża semafor `SEM_LICZNIK_KONCA` (opis w sekcji 2.5).
4. **Dostęp do Pamięci Dzielonej:** Kandydaci z maturą mapują pamięć (`mmap`). W przypadku błędu mapowania proces kończy działanie.
5. **Oczekiwanie na rejestrację:** Kandydat wchodzi w sekcję krytyczną (`mutex_rejestracji`). Sprawdza, czy został wpisany na listę. Jeśli nie, zasypia na zmiennej warunkowej `cond_rejestracji`, czekając na sygnał `broadcast` od Dziekana.

### 2.5. Wewnętrzna Inicjalizacja Procesów Komisji

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
* Watki istnieja tak długo az dziekan ich nie poinformuje o zakończeniu wpuszczania kandydatów

## 2.6. Logika operacyjna Komisji

Procesy komisji realizują kluczową logikę egzaminacyjną. Ich działanie opiera się na ścisłej współpracy wątku Przewodniczącego (zarządcy) oraz wątków Członków (egzaminatorów). Wątki te funkcjonują w nieskończonej pętli, dopóki Dziekan nie zasygnalizuje zakończenia rekrutacji. Po zakończeniu pętli głównej następuje faza sprzątania (`pthread_join`), w której proces czeka na zakończenie wszystkich wątków potomnych.

### 2.6.1. Algorytm Przewodniczącego Komisji I

Wątek Przewodniczącego (PID wątku = 0) pełni rolę zarządcy dla danej instancji komisji. Po zainicjowaniu (pobraniu ID i zwolnieniu argumentów), rozpoczyna on cykliczną pracę w pętli `while(1)`.

**1. Pobranie Kandydata i Zasobów**
Przewodniczący wykonuje sekwencję operacji blokujących na semaforach, co gwarantuje atomowość i poprawną kolejność działań:

* `sem_wait(sem_kolejka_komisji)`: Czeka na pojawienie się kandydata. Domyślna wartość 0 sprawia, że komisja nie zużywa zasobów procesora, gdy kolejka jest pusta.
* `sem_wait(sem_miejsca_komisji)`: Czeka na zwolnienie się fizycznego stanowiska (stolika).

**2. Rezerwacja Stanowiska**
Po przejściu semaforów, Przewodniczący wywołuje funkcję pomocniczą `znajdz_wolne_stanowisko()`, która zwraca indeks wolnego stolika (0-2). Następnie blokuje `mutex` tego konkretnego stanowiska, rezerwując je na wyłączność dla procesu przygotowania egzaminu.

**3. Identyfikacja Kandydata w Pamięci Dzielonej**
Przewodniczący skanuje tablicę studentów w poszukiwaniu rekordu o statusie `1` (Oczekujący na A) lub odpowiednim dla Komisji B. Po znalezieniu:

* Zmienia status na `11` (W trakcie egzaminu), co "wyjmuje" kandydata z puli dostępnych dla innych procesów.
* Pobiera PID kandydata i indeks w tablicy.

*Sytuacja wyjątkowa:* Jeżeli mimo opuszczenia semafora kolejki nie znaleziono kandydata ze statusem `1` (np. wyścig procesów lub błąd logiczny), Przewodniczący zwalnia zasoby (mutex stolika, semafory miejsc i kolejki) i ponawia pętlę (`continue`), aby uniknąć zakleszczenia.

**4. Obsługa "Zaliczenia z Przeszłości" (Tylko Komisja A)**
Przewodniczący weryfikuje flagę `zdana_teoria_wczesniej`. Jeżeli kandydat posiada zaliczenie:

* Blokuje mutex studenta (`mutex_ipc`) dla zachowania spójności danych.
* Ustawia flagę `zaliczona_A = 1` i status `2` (Skierowany do B).
* Zwalnia mutex studenta.
* Podnosi semafor `KOLEJKA_KOMISJA_B` (przekazując kandydata dalej).
* Zwalnia stolik i semafory wejściowe, kończąc obsługę tego przypadku bez angażowania członków komisji.

**5. Przygotowanie Egzaminu**
Dla standardowego kandydata Przewodniczący inicjalizuje strukturę egzaminu w pamięci dzielonej (zeruje liczniki pytań, resetuje tablicę ocen).
Następnie **zwalnia mutex stolika**. Jest to operacja krytyczna – mutex ten musi zostać zwolniony przed wezwaniem członków komisji, aby nie doprowadzić do zakleszczenia (członkowie będą potrzebowali tego mutexa, aby odczytać ID kandydata przypisanego do stolika).

**6. Rekrutacja Zespołu Egzaminacyjnego**
Przewodniczący przystępuje do zebrania członków komisji przy stoliku. Wykorzystuje do tego globalny `mutex_rekrutacja` i zmienną warunkową `cond_wolni_czlonkowie`.

* Blokuje `mutex_rekrutacja`.
* Ustawia w strukturze stolika liczbę `potrzebni_czlonkowie` (4 dla A, 2 dla B).
* Wysyła sygnał `pthread_cond_broadcast`, budząc uśpionych członków komisji.
* Zwalnia `mutex_rekrutacja`.

**7. Synchronizacja na Barierze**
Po wysłaniu wezwania, Przewodniczący (który również pełni rolę egzaminatora) udaje się do "punktu zbiórki", wywołując `pthread_barrier_wait`. Wątek zostaje zablokowany do momentu, aż przy stoliku zbierze się wymagany komplet członków.

### 2.6.2. Algorytm Członka Komisji I

Wątki członków komisji działają jako pula pracowników, oczekująca na zlecenia od Przewodniczącego.

**Mechanizm Oczekiwania i Przydziału:**
Członek komisji wchodzi w pętlę nieskończoną, w której próbuje znaleźć pracę:

1. Blokuje `mutex_rekrutacja`.
2. Sprawdza w pętli wszystkie stanowiska, szukając takiego, gdzie `potrzebni_czlonkowie > 0`.
3. Jeżeli nie znajdzie pracy:

* Wywołuje `pthread_cond_wait(&cond_wolni_czlonkowie, &mutex_rekrutacja)`. Funkcja ta **atomowo zwalnia mutex** i usypia wątek.
* Po otrzymaniu sygnału (broadcast od Przewodniczącego), wątek budzi się i automatycznie ponownie blokuje mutex, wracając do sprawdzania stanowisk.

1. Jeżeli znajdzie pracę (wolny slot przy stoliku):

* Dekrementuje licznik `potrzebni_czlonkowie` (rezerwuje miejsce dla siebie).
* Zapamiętuje numer stolika.
* Przerywa pętlę szukania.

Po zakończeniu tej procedury, wątek przechodzi do realizacji właściwego egzaminu, wywołując funkcję `pracuj_jako_czlonek`.

### 2.6.3. Procedura Egzaminacyjna

Od tego momentu kod i logika są w pełni uniwersalne dla obu typów komisji (oraz dla Przewodniczącego wchodzącego w rolę egzaminatora). Procedura ta realizuje synchroniczne zadawanie pytań.

**1. Synchronizacja na Barierze Startowej**
Na początku funkcji, każdy wątek wykonuje:
`pthread_barrier_wait(&stanowiska[stolik].bariera);`

Zatrzymanie się na tej barierze gwarantuje, że:

* Wszyscy egzaminatorzy są fizycznie gotowi przy stoliku przed rozpoczęciem egzaminu.
* Zaden wątek nie zacznie losować pytań, dopóki komplet komisji się nie zamelduje.

**2. Zadawanie Pytania (Sekcja Krytyczna Stanowiska)**
Po przekroczeniu bariery, wątek blokuje `mutex` przypisany do konkretnego stolika (`stanowiska[stolik].mutex`), aby bezpiecznie zmodyfikować pamięć.

* **Losowanie:** Wywoływana jest funkcja `wylosuj_unikalne_pytanie(stolik)`, która sprawdza historię stolika i zwraca ID pytania, które nie padło wcześniej w tej sesji.
* **Zapis Lokalny:** Pytanie trafia do lokalnej struktury stanowiska, a licznik `liczba_zadanych_pytan` jest inkrementowany.
* **Zapis do Pamięci Dzielonej (IPC):** Wątek identyfikuje indeks kandydata w głównej pamięci (`shm_idx`) na podstawie jego PID i zapisuje:
* Treść pytania w tablicy `pytania[]`.
* Swoje ID w tablicy `id_egzaminatora[]`.

Zapisanie ID egzaminatora (wątku) obok pytania tworzy jawną historię egzaminu, umożliwiającą weryfikację, kto zadał konkretne pytanie. Dzięki mutexowi stolika, zapisy te są atomowe i uporządkowane, mimo równoległego działania wątków.

**3. Synchronizacja na Barierze Końcowej**
Po zadaniu pytania i zwolnieniu mutexa, wątek ponownie oczekuje na barierze. Jest to sygnał dla Przewodniczącego, że **wszyscy** członkowie zakończyli wpisywanie danych i arkusz egzaminacyjny w pamięci dzielonej jest kompletny.

### 2.6.4. Koordynacja Egzaminu przez Przewodniczącego

Przewodniczący komisji, który również brał udział w zadawaniu pytań (wykonując procedurę z pkt 4.6.3 jako jeden z egzaminatorów), po przekroczeniu bariery końcowej przejmuje rolę koordynatora komunikacji ze studentem.

**1. Przekazanie Arkusza (Signal)**
Przewodniczący blokuje indywidualny `mutex_ipc` kandydata w pamięci dzielonej (jest to wymóg techniczny zmiennych warunkowych).
Następnie ustawia flagę `status_arkusza = 1`.

* *Uwaga:* Wartość `1` oznacza w wewnętrznym protokole: "Pytania gotowe, oczekuję na odpowiedź".
Po zmianie statusu wysyła sygnał `pthread_cond_signal`, budząc proces kandydata.

**2. Oczekiwanie na Odpowiedź (Timed Wait)**
Przewodniczący wchodzi w stan oczekiwania, używając funkcji z limitem czasu:
`pthread_cond_timedwait(&cond_ipc, &mutex_ipc, &ts);`

Funkcja ta realizuje trzy zadania w jednym kroku:

* **Odblokowanie Mutexa:** Pozwala kandydatowi (który właśnie się budzi) zablokować mutex i zapisać odpowiedzi.
* **Uśpienie:** Zawiesza wątek Przewodniczącego.
* **Timeout (Fail-safe):** Ustawia limit czasu oczekiwania (czas rzeczywisty + 2 sekundy). Zabezpiecza to system przed zawieszeniem (deadlockiem) w przypadku awarii procesu kandydata.

**3. Weryfikacja Statusu**
Po wybudzeniu Przewodniczący sprawdza wynik operacji:

* Jeżeli `status_arkusza == 2`, oznacza to sukces – kandydat odpowiedział. Rozpoczyna się proces oceniania.
* Jeżeli funkcja zwróciła `ETIMEDOUT` lub status się nie zmienił, kandydat zostaje uznany za nieaktywnego. Przewodniczący ustawia jego globalny status na `3` (Odrzucony) i kończy procedurę.

### 2.6.5. Kandydat odpowiada komisji

Ostatnim etapem interakcji jest odebranie arkusza przez kandydata, rozwiązanie testu i odesłanie odpowiedzi. Proces ten sterowany jest przez zmiany zmiennej `status_arkusza` w pamięci dzielonej.

**Definicja kodów sterujących (`status_arkusza`):**
Aby zapewnić poprawną synchronizację, przyjęto następujący protokół komunikacji:

* `0`: Stan spoczynku (brak akcji).
* `1`: **Pytania Gotowe** (ustawia Przewodniczący, odczytuje Kandydat).
* `2`: **Odpowiedzi Gotowe** (ustawia Kandydat, odczytuje Przewodniczący).

**1. Pętla Oczekiwania na Arkusz**
Kandydat wchodzi w sekcję krytyczną, blokując swój indywidualny `mutex_ipc`. Następnie uruchamia pętlę oczekiwania na zmiennej warunkowej:

```c
while (kandydat->status_arkusza != 1 && kandydat->status != 3) {
    pthread_cond_wait(&kandydat->cond_ipc, &kandydat->mutex_ipc);
}

```

Mechanizm ten jest odporny na fałszywe wybudzenia. Pętla przerywana jest tylko w dwóch przypadkach:

* **Otrzymanie pytań (`status_arkusza == 1`):** Komisja zakończyła wpisywanie pytań.
* **Sygnał zakończenia (`status == 3`):** Kandydat został odrzucony (np. z powodu przekroczenia czasu przez komisję) lub symulacja dobiega końca. W takim przypadku proces zwalnia mutex i kończy działanie.

**2. Dynamiczna Weryfikacja Pytań**
Po otrzymaniu arkusza, kandydat nie polega na sztywnych założeniach dotyczących typu komisji, lecz dynamicznie weryfikuje zawartość pamięci dzielonej. Iteruje on przez tablicę `pytania[5]`, zliczając wszystkie pola zawierające poprawne identyfikatory pytań (wartości `> 0`).

**3. Symulacja Procesu Odpowiadania**
Proces udzielania odpowiedzi symulowany jest poprzez uśpienie procesu na czas obliczony na podstawie stałej `GODZINA_TI` (jednostka czasu symulacji).
W tym czasie kandydat:

* Generuje losowe odpowiedzi dla każdego znalezionego pytania.
* Zapisuje je w tablicy `odpowiedzi[]` w pamięci dzielonej.
* Loguje fakt przystąpienia do egzaminu, podając rzeczywistą liczbę otrzymanych pytań.

**4. Odesłanie Arkusza**
Po zapisaniu wszystkich odpowiedzi, kandydat finalizuje transakcję:

1. Zmienia `status_arkusza` na `2` (sygnał dla Przewodniczącego).
2. Wysyła sygnał `pthread_cond_signal`, budząc wątek Przewodniczącego oczekujący na `timedwait`.
3. Zwalnia `mutex_ipc`.

W tym momencie rola procesu kandydata w sekcji egzaminacyjnej dobiega końca – przechodzi on ponownie w stan oczekiwania (na wynik oceniania lub skierowanie do kolejnej komisji).

### 2.6.6. Analiza odpowiedzi i ocena końcowa

Po upływie czasu przeznaczonego na egzamin (lub otrzymaniu sygnału od kandydata), Przewodniczący komisji przystępuje do weryfikacji i oceny. Proces ten jest ściśle zsynchronizowany z pozostałymi członkami komisji.

**1. Weryfikacja Statusu (Przewodniczący)**
W pierwszej kolejności Przewodniczący sprawdza, czy kandydat udzielił odpowiedzi:

* **Brak odpowiedzi / Timeout:** Jeżeli funkcja oczekująca zwróciła błąd czasowy lub `status_arkusza` pozostał niezmieniony, kandydat uznawany jest za niezdolnego do dalszego procesu. Jego globalny status zmieniany jest na `3` (Odrzucony), podnoszony jest semafor `SEM_LICZNIK_KONCA` (informujący Dziekana o zakończeniu procesu), a procedura oceniania jest pomijana.
* **Odpowiedzi dostępne:** Jeżeli `status_arkusza == 2`, Przewodniczący zwalnia blokadę na barierze, dopuszczając członków komisji do sprawdzania prac.

**2. Równoległe Ocenianie (Członkowie Komisji)**
Wszyscy egzaminatorzy (wątki członków oraz wątek przewodniczącego) przechodzą przez barierę i przystępują do oceny.
Każdy wątek iteruje po tablicy pytań kandydata w pamięci dzielonej i szuka pytań przypisanych do swojego ID (`id_egzaminatora[k] == moj_id`).
Dla każdego znalezionego pytania:

* Generowana jest losowa ocena punktowa z zakresu 0-100.
* Wynik zapisywany jest w tablicy `oceny[]`.
* Operacja jest logowana w systemie, co zapewnia pełną transparentność procesu (można zweryfikować, który wątek wystawił którą ocenę).

Po wystawieniu ocen wątki ponownie zatrzymują się na barierze końcowej, czekając na podliczenie wyników.

**3. Decyzja Końcowa i Routing (Przewodniczący)**
Gdy wszystkie oceny cząstkowe są zapisane, Przewodniczący oblicza średnią arytmetyczną punktów. Następnie podejmuje decyzję o losie kandydata, blokując jego `mutex_ipc` (aby zapobiec przedwczesnemu odczytowi statusu przez kandydata).

#### Scenariusz A: Egzamin ZDANY (Średnia > 30.0)

* **Komisja A:**
* Ustawia flagę `zaliczona_A = 1`.
* Wywołuje funkcję `cleaner_pytan()`, która resetuje tablice pytań i ocen w strukturze kandydata do wartości `-1`. Jest to niezbędne, aby Komisja B otrzymała "czysty arkusz" i nie doszło do błędnego odczytu starych pytań.
* Zmienia status kandydata na `2` (Oczekujący na Komisję B).
* Podnosi semafor `KOLEJKA_KOMISJA_B` (`sem_post`), informując kolejną instancję o nowym studencie.

* **Komisja B:**
* Ustawia flagę `zaliczona_B = 1` oraz `czy_przyjety = 1`.
* Zmienia status kandydata na `3` (Koniec procesu / Przyjęty).
* Podnosi semafor `SEM_LICZNIK_KONCA`, informując Dziekana o sukcesie rekrutacyjnym.

#### Scenariusz B: Egzamin OBLANY

* Niezależnie od komisji, ustawiana jest odpowiednia flaga niezdania (`0`).
* Status kandydata zmieniany jest na `3` (Odrzucony).
* Podnoszony jest semafor `SEM_LICZNIK_KONCA`.

**4. Zakończenie Transakcji i Zwolnienie Zasobów**
Na samym końcu Przewodniczący wykonuje procedurę czyszczenia po obsłużonym studencie:

1. Resetuje `status_arkusza` na `0` (lub `3` w przypadku końca), zamykając cykl komunikacyjny.
2. Wysyła sygnał `pthread_cond_signal` do kandydata.

* Jeśli kandydat zdał w A: Sygnał ten budzi go, by mógł sprawdzić nowy status (`2`) i przejść do kolejki B.
* Jeśli kandydat skończył (zdał B lub oblał): Sygnał ten pozwala mu wyjść z pętli głównej i zakończyć proces.

1. Zwalnia `mutex_ipc` kandydata.
2. Oznacza stolik jako wolny (`zajete = 0`) i podnosi semafor `WOLNE_MIEJSCA_KOMISJA_X`, umożliwiając rozpoczęcie egzaminu dla kolejnej osoby z kolejki.

Tym samym cykl obsługi jednego kandydata przez komisję zostaje zamknięty, a przewodniczący wraca na początek pętli głównej (`while(1)`), oczekując na kolejne zgłoszenia.
Oto kompletna i sformatowana końcowa część dokumentacji, uwzględniająca logikę zarządzania procesami przez Dziekana oraz działanie raportowania.

---

## 2.7 Finał procesu symulacji (Dziekan)

Po rozdystrybuowaniu wszystkich zgłoszeń do komisji, proces Dziekana przechodzi w stan pasywnego nadzorcy (monitora). Jego rola na tym etapie polega na oczekiwaniu na sygnały zakończenia płynące od procesów potomnych oraz na ostatecznym sprzątaniu zasobów systemowych.

### 2.7.1 Synchronizacja zakończenia (Bariera Wyjściowa)

Dziekan nie może zakończyć pracy natychmiast po wysłaniu ostatniego kandydata do Komisji A, ponieważ proces egzaminowania trwa nadal. Aby uniknąć przedwczesnego zamknięcia zasobów IPC (co zabiłoby działające komisje), Dziekan wykorzystuje semafor `SEM_LICZNIK_KONCA`.

```c
for (int i = 0; i < faktyczna_liczba_kandydatow; i++) {
    sem_wait(sem_licznik_konca);
}

```

* **Działanie:** Dziekan wykonuje operację `wait` dokładnie tyle razy, ilu kandydatów wpuścił do systemu.
* **Logika:** Każdy kandydat, niezależnie od losu (zdał/oblał/timeout), przed ostatecznym zakończeniem swojego cyklu życia (przez siebie lub komisję) podnosi ten semafor (`post`).
* **Gwarancja:** Kiedy pętla Dziekana dobiegnie końca, mamy 100% pewności, że w systemie nie ma już aktywnego procesu kandydata, a pamięć dzielona nie jest używana przez studentów.

### 2.7.2 Terminacja Komisji i Sprzątanie Procesów

Po upewnieniu się, że studenci opuścili system, Dziekan przystępuje do zamknięcia procesów Komisji A i B. Ponieważ komisje działają w pętlach nieskończonych (`while(1)`), nie zakończą się same. Dziekan wysyła do nich sygnał `SIGTERM`, korzystając z tablicy PID-ów zapisanych podczas inicjalizacji.

Następnie wykonywana jest pętla `wait(NULL)`, która odbiera statusy zakończenia wszystkich procesów potomnych, zapobiegając ich osieroceniu w systemie operacyjnym.

### 2.7.3 Generowanie Raportów Końcowych

Ostatnim zadaniem merytorycznym jest analiza wyników i stworzenie list rankingowych. Funkcja `generuj_raporty` realizuje to zadanie w sposób bezpieczny dla danych:

1. **Kopiowanie Danych:** Tworzona jest lokalna kopia tablicy studentów (`malloc` + `memcpy` ze struktury `EgzaminPamiecDzielona`). Dzięki temu operacje sortowania nie naruszają oryginalnego układu w pamięci dzielonej (co mogłoby być ryzykowne, gdyby jakiś proces jeszcze tam zaglądał).
2. **Sortowanie Bąbelkowe:** Lokalna tablica jest sortowana malejąco według sumy punktów (Teoria + Praktyka).
3. **Generowanie Plików:**

* `lista_rankingowa.txt`: Zawiera wszystkich kandydatów wraz z ich statusami (ZDAL_PRZYJETY, ZDAL_BRAK_MIEJSC, OBLAL, BRAK_MATURY). Statusy są wyliczane dynamicznie na podstawie flag `zaliczona_A`, `zaliczona_B` oraz limitu przyjęć.
* `lista_przyjetych.txt`: Zawiera tylko najlepsze osoby, które zdały oba etapy oraz ich średnia ocena z teorii i praktyki posortowana mieści się w limicie miejsc (`LIMIT_PRZYJEC`).

### 2.7.4 Zamknięcie Systemu

Na samym końcu funkcja `main` Dziekana zamyka pliki logów, a co najważniejsze – usuwa obiekt pamięci dzielonej z systemu (`shm_unlink`). Jest to krytyczne, aby po zakończeniu programu nie pozostały w RAM-ie "wiszące" segmenty pamięci.

Zakończenie procesu Dziekana (rodzica) jest równoznaczne z końcem całej symulacji. Ponieważ procesy potomne (Komisje, Kandydaci) zostały już wcześniej zakończone i odebrane przez `wait`, system operacyjny pozostaje w stanie czystym.

## 3. Dokumentacja techniczna funkcji w projekcie

### 3.1 Dziekan

#### `handler_smierci_dziecka`

**1. Opis funkcji:**
Funkcja obsługi sygnału `SIGCHLD`. Jest wywoływana automatycznie przez system operacyjny w momencie zakończenia działania dowolnego procesu potomnego. Jej głównym celem jest monitorowanie stabilności systemu poprzez weryfikację, czy zakończony proces był procesem Komisji (krytycznym dla symulacji).

* **Przyjmuje:** `int sig` – numer sygnału (zignorowany w ciele funkcji).
* **Zwraca:** `void` (nic nie zwraca).
* **Działanie:** Sprawdza w pętli (nieblokująco), które procesy potomne się zakończyły. Jeśli wykryje, że zakończony proces posiada PID należący do którejś z Komisji (i nie jest to planowane zakończenie rekrutacji), uznaje to za awarię krytyczną, generuje raporty awaryjne i kończy całą symulację.

#### `handler_ewakuacji`

**1. Opis funkcji:**
Funkcja obsługi sygnałów `SIGINT` (Ctrl+C) oraz `SIGUSR1`. Umożliwia bezpieczne, przedwczesne przerwanie symulacji przez użytkownika (symulacja ewakuacji).

* **Przyjmuje:** `int sig` – numer sygnału.
* **Zwraca:** `void`.
* **Działanie:** Wypisuje komunikat o ewakuacji, ustawia flagę blokującą handler awarii, generuje raporty na podstawie aktualnego stanu pamięci dzielonej, a następnie bezpiecznie zabija wszystkie procesy w grupie i kończy program.

---

#### `generuj_raporty`

**1. Opis funkcji:**
Funkcja odpowiedzialna za przetworzenie danych z pamięci dzielonej i wygenerowanie plików wynikowych: listy rankingowej oraz listy przyjętych.

* **Przyjmuje:** Brak argumentów (korzysta ze wskaźnika globalnego `egzamin`).
* **Zwraca:** `void`.
* **Działanie:**

1. Kopiuje dane studentów z pamięci dzielonej do lokalnej tablicy (aby nie blokować pamięci na czas sortowania).
2. Sortuje studentów malejąco według sumy punktów (Teoria + Praktyka).
3. Tworzy plik `lista_rankingowa.txt` zawierający wszystkich kandydatów wraz z ich ostatecznym statusem (np. OBLAL, BRAK_MATURY, PRZYJETY).
4. Tworzy plik `lista_przyjetych.txt` zawierający tylko osoby, które zdały oba etapy i zmieściły się w limicie przyjęć.

**2. Pseudokod:**

```text
Funkcja generuj_raporty():
    Jeżeli brak dostępu do pamięci dzielonej -> Wróć
    
    Utwórz lokalną kopię tablicy studentów
    Posortuj kopię malejąco wg sumy punktów (Sortowanie Bąbelkowe)
    
    Otwórz plik "lista_rankingowa.txt":
        Dla każdego studenta w posortowanej liście:
            Oblicz status (Brak matury / Oblał / Przyjęty / Brak miejsc / Ewakuacja)
            Zapisz wiersz z danymi i statusem do pliku
            
    Otwórz plik "lista_przyjetych.txt":
        Dla każdego studenta w posortowanej liście:
            Jeżeli (Zaliczył A i Zaliczył B) ORAZ (licznik < LIMIT_PRZYJĘĆ):
                Zapisz wiersz z danymi do pliku
                Zwiększ licznik przyjętych
                
    Zwolnij pamięć lokalnej kopii

```

### 3.2 Komisja

#### `cleaner_pytan`

**1. Opis funkcji:**
Funkcja pomocnicza służąca do resetowania tablic pytań, odpowiedzi, ocen i identyfikatorów egzaminatorów w strukturze studenta w pamięci dzielonej. Jest używana głównie przed wysłaniem kandydata do kolejnej komisji, aby usunąć dane z poprzedniego etapu (np. pytania z Komisji A nie mogą być widoczne w Komisji B).

* **Przyjmuje:** `int id_kandydata` – indeks studenta w tablicy pamięci dzielonej.
* **Zwraca:** `void`.
* **Działanie:** Iteruje po 5-elementowych tablicach w strukturze studenta i ustawia ich wartości na `-1` (wartość neutralna/pusta).

---

#### `znajdz_wolne_stanowisko`

**1. Opis funkcji:**
Funkcja przeszukująca lokalną tablicę stanowisk (stolików) w celu znalezienia takiego, które jest aktualnie wolne.

* **Przyjmuje:** Brak argumentów.
* **Zwraca:** `int` – indeks wolnego stanowiska (0..MAX_MIEJSC-1) lub `-1`, jeśli wszystkie są zajęte.
* **Działanie:** W sposób bezpieczny wątkowo (zamykając mutex każdego stanowiska) sprawdza flagę `zajete`. Jeśli znajdzie wolne miejsce, natychmiast zmienia flagę na `1` (rezerwacja) i zwraca indeks.

---

#### `wylosuj_unikalne_pytanie`

**1. Opis funkcji:**
Generuje losowy numer pytania, dbając o to, aby przy danym stoliku to samo pytanie nie padło dwukrotnie podczas jednego egzaminu.

* **Przyjmuje:** `int stolik_idx` – indeks stanowiska, dla którego losujemy pytanie.
* **Zwraca:** `int` – unikalny numer pytania (1-50).
* **Działanie:** W pętli `while` losuje liczbę. Następnie sprawdza historię zadanych pytań przy tym stoliku (`stanowiska[stolik].pytania`). Jeśli wylosowana liczba się powtarza, losuje ponownie.

---

#### `pracuj_jako_czlonek`

**1. Opis funkcji:**
Uniwersalna funkcja realizująca logikę zadawania pytania przez członka komisji (zarówno A, jak i B).

* **Przyjmuje:** * `stolik` – numer przypisanego stanowiska.
* `id` – ID egzaminatora.
* `typ_egzaminatora` – rola ('P' lub 'C').
* `typ_komisji` – 'A' lub 'B'.

* **Zwraca:** `void`.
* **Działanie:**

1. Czeka na `barierze` startowej (synchronizacja całego zespołu przy stole).
2. W sekcji krytycznej losuje pytanie i zapisuje je w pamięci dzielonej kandydata.
3. Loguje fakt zadania pytania.
4. Czeka na `barierze` kończącej fazę zadawania pytań.

**2. Pseudokod:**

```text
Funkcja pracuj_jako_czlonek(...):
    Czekaj na barierze (start egzaminu)
    
    Zablokuj mutex stanowiska
    Wylosuj unikalne pytanie
    Zapisz pytanie i ID egzaminatora w Pamięci Dzielonej studenta
    Dodaj wpis do logów (Zadano pytanie)
    Odblokuj mutex stanowiska
    
    Czekaj na barierze (koniec zadawania pytań)

```

---

#### `czlonek_komisji_A` / `czlonek_komisji_B`

(Funkcje te są bliźniacze, różnią się detalami logowania i liczbą członków, dlatego opisano je łącznie)

**1. Opis funkcji:**
Funkcja wątku realizująca zadania szeregowego członka komisji.

* **Przyjmuje:** `void* arg` – wskaźnik na ID członka.
* **Zwraca:** `void*`.
* **Działanie:**

1. W pętli nieskończonej szuka pracy: sprawdza, czy jakieś stanowisko potrzebuje członków (`potrzebni_czlonkowie > 0`). Jeśli nie, zasypia na zmiennej warunkowej `cond_wolni_czlonkowie`.
2. Po przydzieleniu do stolika wykonuje `pracuj_jako_czlonek` (zadaje pytanie).
3. Czeka na barierze, aż student odpowie.
4. Losuje ocenę (0-100), jeśli student odpowiedział, i zapisuje ją w pamięci dzielonej.
5. Czeka na barierze kończącej ocenianie i wraca do puli wolnych członków.

**2. Pseudokod:**

```text
Funkcja czlonek_komisji_X(arg):
    Pętla nieskończona:
        Zablokuj mutex_rekrutacja
        Dopóki brak wolnego stolika:
            Czekaj na cond_wolni_czlonkowie
        Zdekremnetuj potrzebni_czlonkowie, przypisz się do stolika
        Odblokuj mutex_rekrutacja
        
        Wywołaj pracuj_jako_czlonek() // Zadawanie pytania
        
        Czekaj na barierze (aż student odpowie)
        
        Jeżeli student odpowiedział (status_arkusza == 2):
            Wylosuj ocenę (0-100)
            Zapisz ocenę w Pamięci Dzielonej
            Zaloguj ocenę
            
        Czekaj na barierze (koniec oceniania)

```

#### `przewodniczacy_komisji_A` / `przewodniczacy_komisji_B`

**1. Opis funkcji:**
Funkcja realizująca logikę wątku Przewodniczącego. Jest to centralna postać w procesie egzaminowania – zarządza pobieraniem studentów z kolejki, synchronizuje pracę członków komisji przy użyciu barier oraz odpowiada za komunikację IPC z procesem kandydata.

* **Przyjmuje:** `void* arg` – wskaźnik na ID przewodniczącego (zazwyczaj 0).
* **Zwraca:** `void*`.

**Wspólny schemat działania:**

1. **Pobranie kandydata:** Oczekiwanie na semaforach kolejki (odpowiednio A lub B) oraz wolnych miejsc w sali.
2. **Organizacja zespołu:** Wyszukanie wolnego stolika i obudzenie wymaganej liczby członków komisji (`cond_wolni_czlonkowie`).
3. **Egzamin:** Przeprowadzenie egzaminu z użyciem barier synchronizacyjnych (Start -> Zadawanie pytań -> Ocenianie).
4. **Komunikacja IPC:** Wysłanie sygnału do kandydata i oczekiwanie na jego odpowiedź z limitem czasu (**Timeout**). Brak odpowiedzi w zadanym czasie skutkuje dyskwalifikacją.
5. **Decyzja:** Obliczenie średniej punktów i skierowanie kandydata dalej.

**Kluczowe różnice w logice:**

* **Komisja A (Selekcja wstępna):**
* Sprawdza flagę `zdana_teoria_wczesniej`. Jeśli kandydat zdał teorię w latach ubiegłych, przewodniczący **pomija egzamin**, natychmiast zalicza etap A i przesyła studenta do kolejki B.
* W przypadku zdania egzaminu: Przesyła kandydata do Komisji B (`sem_post(KOLEJKA_B)`).
* W przypadku porażki: Kończy proces kandydata (`sem_post(LICZNIK_KONCA)`).

* **Komisja B (Finalizacja):**
* Nie sprawdza historii teorii (każdy musi zdać praktykę).
* W przypadku zdania egzaminu: Ustawia flagę `czy_przyjety = 1`.
* **Zawsze** kończy proces kandydata (niezależnie czy zdał, czy nie), podnosząc semafor `LICZNIK_KONCA`, co jest sygnałem dla Dziekana o zakończeniu obsługi tego studenta.

**2. Pseudokod (Zintegrowany):**

```text
Funkcja przewodniczacy_komisji_X(arg):
    Pętla nieskończona:
        // 1. Oczekiwanie na zasoby
        Jeżeli Komisja A: Czekaj(KOLEJKA_A), Czekaj(MIEJSCA_A)
        Jeżeli Komisja B: Czekaj(KOLEJKA_B), Czekaj(MIEJSCA_B)
        
        Znajdź wolny stolik (Mutex)
        Pobierz ID kandydata z pamięci dzielonej (A: status==1, B: status==2)
        
        // --- LOGIKA SPECYFICZNA DLA A (BYPASS) ---
        Jeżeli (Komisja == A) ORAZ (kandydat.zdana_teoria_wczesniej == 1):
            Ustaw kandydat.zaliczona_A = 1
            Ustaw kandydat.status = 2
            Wyczyść pytania
            Podnieś semafor KOLEJKA_B
            Zwolnij stolik i semafor MIEJSCA_A
            CONTINUE (Przejdź do następnego studenta)
            
        // 2. Rekrutacja Zespołu
        Zablokuj Mutex Rekrutacji
        Ustaw potrzebni_czlonkowie = N
        Broadcast(cond_wolni_czlonkowie)
        Odblokuj Mutex
        
        // 3. Egzamin (Faza Pytań) 
        Czekaj na Barierze (Start)
        Zadaj pytanie (Wylosuj, Zapisz w SHM, Zaloguj)
        Czekaj na Barierze (Pytania Zadane)
        
        // 4. Komunikacja IPC z Timeoutem
        Zablokuj Mutex Studenta (mutex_ipc)
        Ustaw status_arkusza = 1 (Masz pytania)
        Signal(cond_ipc_studenta) // Obudź studenta
        
        Ustaw czas_oczekiwania = TERAZ + 2 sekundy
        
        Dopóki (status_arkusza != 2 ORAZ wynik != TIMEOUT):
            wynik = TimedWait(cond_ipc_studenta, mutex_ipc, czas_oczekiwania)
            
        Jeżeli wynik == TIMEOUT:
            Zaloguj "Dyskwalifikacja"
            Ustaw kandydat.status = 3 (Oblał)
            Signal(cond_ipc_studenta) // Poinformuj o końcu
            
        Odblokuj Mutex Studenta
        
        // 5. Egzamin (Faza Oceniania)
        Czekaj na Barierze (Pozwolenie na ocenianie)
        Wystaw swoją ocenę (jeśli nie było timeoutu)
        Czekaj na Barierze (Koniec oceniania)
        
        // 6. Podsumowanie
        Oblicz średnią ocen ze stolika
        
        Jeżeli (średnia >= 30) ORAZ (nie było timeoutu):
            // --- ZDANE ---
            Jeżeli Komisja A:
                Ustaw zaliczona_A = 1, status = 2
                Podnieś semafor KOLEJKA_B
            Jeżeli Komisja B:
                Ustaw zaliczona_B = 1, czy_przyjety = 1, status = 3
                Podnieś semafor LICZNIK_KONCA
        Inaczej:
            // --- OBLANE ---
            Ustaw zaliczona_X = 0, status = 3
            Podnieś semafor LICZNIK_KONCA (Dla A i B tak samo)
            
        // 7. Sprzątanie
        Zresetuj status_arkusza
        Signal(cond_ipc_studenta) // Zwolnij proces kandydata
        Zwolnij stolik
        Podnieś semafor MIEJSCA_X

```

---

### 3.3 Moduł Narzędziowy (Common)

#### `sleep_ms`

**1. Opis funkcji:**
Funkcja pomocnicza realizująca precyzyjne wstrzymanie wykonania wątku na określoną liczbę milisekund. Stanowi wrapper na systemową funkcję `nanosleep`, która jest bezpieczna w środowisku wielowątkowym i nie interferuje z sygnałami tak jak starsze `sleep()`.

* **Przyjmuje:** `int ms` – czas oczekiwania w milisekundach.
* **Zwraca:** `void`.
* **Działanie:** Konwertuje milisekundy na strukturę `timespec` (sekundy i nanosekundy), a następnie wywołuje funkcję systemową.

---

#### `utworz_folder`

**1. Opis funkcji:**
Sprawdza istnienie katalogu o podanej ścieżce i tworzy go, jeśli nie istnieje.

* **Przyjmuje:** `const char *sciezka` – ścieżka do folderu (np. "logi").
* **Zwraca:** `void`.
* **Działanie:** Wykorzystuje funkcję `stat` do sprawdzenia metadanych pliku. Jeśli funkcja zwróci błąd (oznaczający brak pliku/katalogu), następuje próba utworzenia katalogu funkcją `mkdir` z pełnymi uprawnieniami (0777).

---

#### `pobierz_czas`

**1. Opis funkcji:**
Pobiera aktualny czas systemowy z precyzją do milisekund i formatuje go do czytelnego łańcucha znaków.

* **Przyjmuje:** * `char *buffer` – bufor na wynikowy napis.
* `size_t size` – rozmiar bufora.


* **Zwraca:** `void`.
* **Działanie:** Pobiera czas za pomocą `gettimeofday`, rozbija go na strukturę czasu lokalnego (`localtime`), a następnie formatuje do postaci `GG:MM:SS.ms`.

---

#### `otworz_log`

**1. Opis funkcji:**
Tworzy i otwiera plik logu z unikalną nazwą zawierającą datę i godzinę uruchomienia. Zapewnia to, że każde uruchomienie symulacji generuje oddzielny plik historii.

* **Przyjmuje:**
* `nazwa_bazowa` – identyfikator pliku (np. "dziekan", "komisja_A").
* `tryb` – tryb otwarcia pliku (np. "w", "a").

* **Zwraca:** `FILE*` – wskaźnik na otwarty strumień pliku lub `NULL` w przypadku błędu.
* **Działanie:** 1. Upewnia się, że folder "logi" istnieje.

1. Generuje pełną nazwę pliku w formacie: `logi/RRRR-MM-DD_GG-MM-SS_logi_[nazwa].txt`.
2. Otwiera plik i zwraca jego uchwyt.

**2. Pseudokod:**

```text
Funkcja otworz_log(nazwa, tryb):
    Wywołaj utworz_folder("logi")
    
    Jeżeli nazwa zawiera już ścieżkę:
        Użyj podanej nazwy
    Inaczej:
        Pobierz aktualną datę
        Sformatuj nazwę: "logi/DATA_GODZINA_logi_NAZWA.txt"
        
    Otwórz plik (fopen)
    Zwróć wskaźnik pliku

```

---

#### `dodaj_do_loggera`

**1. Opis funkcji:**
Główna funkcja logująca, obsługująca zmienną liczbę argumentów. Realizuje tzw. **podwójne logowanie**: wypisuje komunikaty na standardowe wyjście (konsolę) z użyciem kodów kolorów ANSI oraz zapisuje te same komunikaty (w formie czystego tekstu) do pliku.

* **Przyjmuje:**
* `FILE *plik` – uchwyt do pliku logu (może być NULL).
* `const char *format` – format wiadomości (jak w printf).
* `...` – zmienna lista argumentów.

* **Zwraca:** `void`.
* **Działanie:**

1. Pobiera sformatowany czas.
2. **Konsola:** Wypisuje czas i treść komunikatu, poprzedzając go globalnie ustawionym kolorem (`AKTUALNY_KOLOR_LOGU`), a na końcu resetuje kolor.
3. **Plik:** Jeśli podano uchwyt pliku, zapisuje do niego ten sam komunikat (wraz z czasem), ale bez kodów sterujących kolorami.
4. Wymusza opróżnienie buforów (`fflush`), aby logi pojawiały się natychmiastowo.

**2. Pseudokod:**

```text
Funkcja dodaj_do_loggera(plik, format, ...):
    Pobierz aktualny czas (pobierz_czas)
    
    // Wyjście na ekran
    Wypisz KOLOR + Czas
    Wypisz sformatowany komunikat (vprintf)
    Zresetuj KOLOR
    Opróżnij bufor (fflush stdout)
    
    // Wyjście do pliku
    Jeżeli plik istnieje:
        Zapisz Czas do pliku
        Zapisz sformatowany komunikat do pliku (vfprintf)
        Opróżnij bufor pliku (fflush)

```

## 4. Testy

### 4.1. Test scenariusza awarii: Nagłe zakończenie procesu kandydata (`SIGKILL`)

**Cel testu:**
Weryfikacja odporności systemu na krytyczną awarię procesu potomnego (Kandydata) w trakcie trwania egzaminu. Test ma na celu potwierdzenie, że Komisja nie ulegnie zakleszczeniu (deadlock) w oczekiwaniu na odpowiedź nieistniejącego procesu, lecz obsłuży wyjątek poprzez mechanizm `TIMEOUT`.

**Metodyka:**

1. Uruchomienie symulacji i identyfikacja PID aktywnego kandydata.
2. Zewnętrzne wymuszenie zakończenia procesu kandydata za pomocą sygnału `kill -9 [PID]` (SIGKILL) w momencie, gdy przebywa on w sali egzaminacyjnej (po otrzymaniu pytań, przed wysłaniem odpowiedzi).
3. Analiza logów pod kątem wystąpienia błędu przekroczenia czasu oczekiwania (Timeout) i poprawności zwolnienia zasobów.

### 4.1.1. Weryfikacja utworzenia procesu

Na podstawie logów systemowych potwierdzono, że kandydat o numerze PID `183887` został poprawnie utworzony, zweryfikowany przez Dziekana i skierowany do Komisji A.

```text
[Czas: 10:21:51.157]    [Kandydat] [PID: 183887]     Zostałem utworzony i mam zdaną mature.
[Dziekan] [PID: 183477] Kandydat [PID: 183887] skierowany do komisji A [PID: 183478]

```

### 4.1.2. Symulacja awarii

Wysłanie sygnału `SIGKILL` do procesu kandydata z poziomu terminala. Proces zostaje natychmiastowo usunięty z systemu operacyjnego, nie mając szansy na wysłanie odpowiedzi do pamięci dzielonej ani podniesienie semaforów/zmiennych warunkowych.

5.1. Wymuszenie zamknięcia procesu kandydata poleceniem "kill -9 PID_KANDyDATA"

```bash
$ kill -9 183887
```

### 4.1.3. Reakcja Komisji A (Obsługa Timeoutu)

Analiza logów wykazuje poprawną reakcję systemu. Egzaminatorzy zadali pytania, po czym proces Komisji przeszedł w stan oczekiwania (`pthread_cond_timedwait`). Z powodu braku sygnału zwrotnego od zabitego kandydata, upłynął zdefiniowany czas oczekiwania.

```text

[Czas: 10:23:50.029]    [Komisja A] [PID: 183478] |  Pytanie nr: 1 |     Egzaminator [P] [TID: 183481] [ID: 0] |     Zadał kandydatowi [PID: 183887] treść nr: 18
...
[Czas: 10:23:50.029]    [Komisja A] [PID: 183478] |  Pytanie nr: 5 |     Egzaminator [C] [TID: 183488] [ID: 4] |     Zadał kandydatowi [PID: 183887] treść nr: 38

[Czas: 10:23:52.031]    [Komisja A] [PID: 183478] TIMEOUT! Kandydat [PID: 183887] nie odpowiedział. Dyskwalifikacja.

```

**Analiza wyniku:**
Mechanizm zabezpieczający zadziałał poprawnie. Po wykryciu `ETIMEDOUT`:

1. System uznał kandydata za zdyskwalifikowanego.
2. Odpowiedzi zostały automatycznie ocenione jako `0` (brak danych w pamięci dzielonej).
3. Wynik końcowy wyniósł `0.00 pkt`, co skutkowało negatywną oceną ("OBLAŁ A").

```text

[Czas: 10:23:52.031]    [Komisja A] [PID: 183478] |  Egzaminator [C] [TID: 183488] [ID: 4] |     Ocenił kandydata [PID: 183887] | Numer pytania: 38 | Jego odpowiedz: 0      Wynik cząstkowy: 0
...
[Czas: 10:23:52.032]    [Komisja A] [PID: 183478] |  Kandydat [PID: 183887] |    OBLAŁ A (Koniec) |  Wynik: 0.00 pkt

```

### 4.1.4. Weryfikacja izolacji błędu (Komisja B)

Sprawdzono logi Komisji B. Zgodnie z oczekiwaniami, logi **nie zawierają żadnych wpisów** dotyczących kandydata `183887`.

**Wniosek:**

Logika przepływu została zachowana. Kandydat, który nie zdał egzaminu w Komisji A (nawet w wyniku awarii/timeoutu), nie został przekazany do kolejnego etapu. Awaria została odizolowana i nie wpłynęła na stabilność pracy Komisji B ani Dziekana. System poprawnie zwolnił zasoby (miejsce w sali A) i kontynuował pracę.

---

## 4.2. Scenariusz testowy: Całkowity brak uprawnień (100% niezdanych matur)

### 4.2.1 Cel testu

Weryfikacja zachowania systemu w warunkach skrajnych, w których żaden z kandydatów nie spełnia wymogów formalnych (brak zdanej matury). Test ma na celu potwierdzenie, że:

1. Mechanizm filtracji wstępnej w procesie Dziekana działa poprawnie.
2. Zasoby systemowe (Komisje, pamięć dzielona dla egzaminów) nie są angażowane niepotrzebnie.
3. Listy wynikowe poprawnie odzwierciedlają przyczynę odrzucenia.

**Konfiguracja środowiska:**
W pliku `common.h` ustawiono parametry eliminujące szansę na sukces kandydata. Dodatkowo wyzerowano szansę na zdaną teorię, aby zachować spójność logiczną (nie można mieć zdanej teorii z lat ubiegłych bez posiadania matury).

```c
#define LICZBA_KANDYDATOW 1000      // Próba badawcza
#define SZANSA_NA_BRAK_MATURY 100   // 100% kandydatów nie zdaje matury
#define SZANSA_NA_ZDANA_TEORIE 0    // Spójność logiczna

```

### 4.2.2 Analiza przebiegu symulacji

**1. Logi Procesów Kandydatów**
Zgodnie z założeniami, każdy utworzony proces kandydata po wylosowaniu atrybutów stwierdza brak uprawnień. Kandydaci wysyłają zgłoszenie do kolejki FIFO i natychmiast kończą działanie, nie próbując nawet mapować pamięci dzielonej egzaminu.

```text
[Czas: 11:02:04.630]    [Kandydat] [PID: 207087]     Zostałem utworzony i nie mam zdanej matury.
[Czas: 11:02:04.632]    [Kandydat] [PID: 207096]     Zostałem utworzony i nie mam zdanej matury.
...
[Czas: 11:02:04.638]    [Kandydat] [PID: 207101]     Zostałem utworzony i nie mam zdanej matury.

```

**2. Logi Procesu Dziekana (Filtracja)**
Dziekan poprawnie odbiera zgłoszenia z potoku nazwanego. Dla każdego z 1000 przypadków system identyfikuje flagę `zdana_matura == 0`. Status kandydata nie jest zmieniany na `1` (skierowany do komisji), lecz jest on natychmiast odrzucany.

```text
[Czas: 11:02:10.988]    [Dziekan] [PID: 207084] Godzina T wybija! Odbieram zgłoszenia (FIFO).
[Czas: 11:02:10.988]    [Dziekan] [PID: 207084] Kandydat [PID: 207087] zarejestrowany w systemie, ale odrzucony (Brak Matury).
[Czas: 11:02:10.988]    [Dziekan] [PID: 207084] Kandydat [PID: 207096] zarejestrowany w systemie, ale odrzucony (Brak Matury).
...
[Czas: 11:02:10.989]    [Dziekan] [PID: 207084] Kandydat [PID: 207102] zarejestrowany w systemie, ale odrzucony (Brak Matury).

```

**3. Logi Podsystemu Komisji (Brak Obciążenia)**
Jest to kluczowy wynik testu. Procesy Komisji A i B zostały poprawnie zainicjowane przez Dziekana, jednak **żaden z nich nie podjął pracy**. Przewodniczący zatrzymali się na semaforach wejściowych (`sem_wait(sem_kolejka_komisji)`), ponieważ Dziekan ani razu nie podniósł semafora `KOLEJKA_KOMISJA_A`. Świadczy to o optymalnym zarządzaniu zasobami CPU – wątki egzaminatorów pozostawały w stanie uśpienia.

```text
[Czas: 11:02:04.630]    [Komisja A] [P] Przewodniczący 0 w komisji A: Rozpoczynam pracę.
[Czas: 11:02:04.630]    [Komisja B] [P] Przewodniczący 0 w komisji B: Rozpoczynam pracę.
// Brak dalszych logów - oczekiwane zachowanie.

```

### 4.2.3 Analiza Raportów Końcowych

Wygenerowane pliki potwierdzają, że system poprawnie przetworzył dane wszystkich 1000 kandydatów, nie gubiąc żadnego rekordu.

**Plik: `lista_rankingowa.txt**`
Wszyscy kandydaci widnieją na liście z zerowym dorobkiem punktowym. Status `BRAK_MATURY` został poprawnie przypisany na podstawie analizy flagi w strukturze studenta.

```text
PID    | OCENA A | OCENA B | OGOLNA OCENA | STATUS
207087 | 0.00    | 0.00    | 0.00         | BRAK_MATURY
207096 | 0.00    | 0.00    | 0.00         | BRAK_MATURY
...
208106 | 0.00    | 0.00    | 0.00         | BRAK_MATURY
208107 | 0.00    | 0.00    | 0.00         | BRAK_MATURY

```

**Plik: `lista_przyjetych.txt**`
Zgodnie z oczekiwaniami, plik jest pusty (zawiera tylko nagłówek). Żaden kandydat nie spełnił warunków rekrutacji.

```text
PID | OCENA A | OCENA B | OGOLNA OCENA

```

### 4.2.4. Wniosek

Test zakończył się powodzeniem. System wykazał się odpornością na skrajnie negatywne dane wejściowe. Mechanizm "sita" na poziomie Dziekana zadziałał poprawnie, zapobiegając przeciążeniu komisji i niepotrzebnym operacjom na pamięci dzielonej. Logika raportowania bezbłędnie zidentyfikowała przyczynę odrzucenia dla całej populacji testowej.

---

## 4.3. Scenariusz testowy: Zaliczenie teorii w latach ubiegłych

### 4.3.1. Cel testu

Scenariusz ma na celu potwierdzenie, że kandydaci posiadający zaliczenie z teorii są automatycznie przekierowywani do Komisji B (egzamin praktyczny), z pominięciem procedury zadawania pytań w Komisji A. Dodatkowo weryfikowana jest poprawność przypisywania ocen archiwalnych (z symulacji "poprzednich lat").

**Konfiguracja środowiska:**
W pliku `common.h` ustawiono parametry gwarantujące, że każdy z 1000 kandydatów posiada zdaną maturę oraz zaliczoną teorię.

```c
#define LICZBA_KANDYDATOW 1000
#define SZANSA_NA_BRAK_MATURY 0     // Wszyscy mają maturę
#define SZANSA_NA_ZDANA_TEORIE 100  // Wszyscy zdali teorię wcześniej

```

### 4.3.2. Analiza przebiegu symulacji

**1. Logika Przekierowania w Komisji A**
Logi potwierdzają, że Komisja A poprawnie identyfikuje status kandydata ("Stary Student"). Zamiast przeprowadzać egzamin, Przewodniczący A natychmiast zmienia status kandydata, zalicza mu etap teoretyczny i przekazuje go do kolejki Komisji B.

```text
[Czas: 11:14:35.400]    [Komisja A] [PID: 210953] |  Kandydat [PID: 210955] (Stary) |    ZALICZONA A (Wcześniej) |   Przekierowanie do B
[Czas: 11:14:35.631]    [Komisja A] [PID: 210953] |  Kandydat [PID: 210963] (Stary) |    ZALICZONA A (Wcześniej) |   Przekierowanie do B

```

```bash
$ cat "data_logu"_logi_komisja_A.log.txt | grep "ZALICZONA A" | wc -l 
1000

```

*Wniosek:* Mechanizm pomijania egzaminu teoretycznego działa bezbłędnie. Komisja A działa jedynie jako punkt komunikacyjny.

**2. Praca Komisji B (Egzamin Praktyczny)**
Kandydaci trafiający do Komisji B są traktowani standardowo. Przewodniczący oraz członkowie zadają po jednym pytaniu (łącznie 3), a następnie oceniają odpowiedzi.

```text
[Czas: 11:14:35.402]    [Komisja B] [PID: 210954] |  Pytanie nr: 1 |     Egzaminator [C] [TID: 210958] [ID: 1] |     Zadał kandydatowi [PID: 210955] treść nr: 15
...
[Czas: 11:14:35.729]    [Komisja B] [PID: 210954] |  Kandydat [PID: 210955] |    ZDAŁ B (Przyjęty) |     Wynik: 48.67 pkt

```

**3. Weryfikacja liczby zadanych pytań (Analiza logów kandydata)**
Aby ostatecznie potwierdzić, że żaden kandydat nie był egzaminowany przez Komisję A (która zadaje 5 pytań), przeprowadzono analizę statystyczną logów przy użyciu narzędzia `grep`.

Wyniki analizy:

* Liczba kandydatów, którzy otrzymali 3 pytania (tylko Komisja B): **1000**
* Liczba kandydatów, którzy otrzymali 5 pytań (Komisja A): **0**

```bash
$ cat "data_logu"_logi_kandydaci.txt | grep "Otrzymałem 3 pytań!" | wc -l
1000
$ cat "data_logu"_logi_kandydaci.txt | grep "Otrzymałem 5 pytań!" | wc -l
0

```

Powyższe dane stanowią dowód matematyczny, że 100% populacji testowej pomyślnie ominęło etap teoretyczny.

### 4.3.3. Analiza Wyników Końcowych

Listy rankingowe odzwierciedlają specyfikę testu. Oceny z "Komisji A" są widoczne w raportach, mimo że egzamin się nie odbył. Wynika to z faktu, że dla kandydatów z zaliczoną teorią system losuje ocenę archiwalną z wyższego przedziału (symulacja dobrych wyników z poprzednich lat: `rand() % 71 + 30`).

**Fragment `lista_przyjetych.txt`:**

```text
PID    | OCENA A | OCENA B | OGOLNA OCENA
211457 | 96.00   | 93.33   | 94.66
211716 | 95.00   | 86.67   | 90.84
...
211025 | 80.00   | 88.67   | 84.34

```

Wysokie noty z części A (powyżej 80 pkt) są zgodne z algorytmem generowania ocen dla "starych studentów" i potwierdzają poprawność migracji danych do raportu końcowego.

### 4.3.4 Wniosek

Test 4.3 zakończył się pełnym sukcesem. System poprawnie obsłużył ścieżke, optymalizując czas przetwarzania kandydatów (brak oczekiwania na 5 pytań w Komisji A). Spójność danych między procesami (Komisja A przekazuje, Komisja B odbiera) została zachowana, a raporty końcowe prawidłowo uwzględniły oceny historyczne (uzupełnione przez proces Dziekana).

---

## 4.4. Weryfikacja mechanizmu limitów przyjęć (Numerus Clausus)

Celem tej serii testów nie jest sprawdzenie przebiegu samego egzaminu (komisje pracują standardowo), lecz weryfikacja logiki post-processingu w procesie Dziekana. Testy mają na celu potwierdzenie, że funkcja `generuj_raporty()` poprawnie sortuje kandydatów i odcina osoby, które zdały egzamin, ale nie zmieściły się w limicie miejsc.

### 4.4.1 Scenariusz A: Ekstremalna selekcja (Tylko 1 miejsce)

**Konfiguracja środowiska:**
Test przeprowadzono na pełnej próbie 1000 kandydatów przy standardowych szansach na zdanie matury i teorii. Zmodyfikowano jednak współczynnik chętnych na miejsce, drastycznie zaostrzając kryteria przyjęcia.

```c
#define LICZBA_KANDYDATOW 1000
#define CHETNI_NA_MIEJSCE 1000
// Wynik: LIMIT_PRZYJEC = 1
#define LIMIT_PRZYJEC (LICZBA_KANDYDATOW / CHETNI_NA_MIEJSCE)

```

**Analiza wyników:**
Symulacja przebiegła bez zakłóceń. Komisje A i B przeegzaminowały wszystkich uprawnionych kandydatów. Kluczowe rozstrzygnięcie nastąpiło na etapie generowania raportów.

1. **Lista Przyjętych:**

Zgodnie z założeniem, na liście znalazł się wyłącznie jeden kandydat z najwyższą sumą punktów.

```text
PID    | OCENA A | OCENA B | OGOLNA OCENA
316885 | 96.00   | 74.00   | 85.00

```

1. **Lista Rankingowa i Statystyki:**

Analiza pliku `lista_rankingowa.txt` przy użyciu polecenia `grep` potwierdza poprawność algorytmu selekcji.

* Liczba osób przyjętych (`ZDAL_EGZAMIN_PRZYJETY`): **1**
* Liczba osób, które zdały, ale się nie dostały (`ZDAL_EGZAMIN_BRAK_MIEJSC`): **811**

```bash

$ cat lista_rankingowa.txt | grep "ZDAL_EGZAMIN_PRZYJETY" | wc -l
1
$ cat lista_rankingowa.txt | grep "ZDAL_EGZAMIN_BRAK_MIEJSC" | wc -l
811

```

**Wniosek A:**
System poprawnie zidentyfikował lidera rankingu (zastosowano sortowanie malejące) i nadał mu status przyjętego. Pozostałe 811 osób, mimo uzyskania pozytywnych ocen z obu egzaminów, otrzymało poprawny status "BRAK MIEJSC". Potwierdza to, że praca komisji (ocenianie) jest logicznie odseparowana od decyzji administracyjnej o przyjęciu.

---

### 4.4.2. Scenariusz B: Zamknięta rekrutacja (0 miejsc)

**Konfiguracja środowiska:**
W tym scenariuszu symulowano sytuację, w której uczelnia przeprowadza egzaminy, ale nie oferuje żadnych miejsc (limit przyjęć ustawiony "na sztywno" na 0).

```c
#define LICZBA_KANDYDATOW 1000
#define LIMIT_PRZYJEC 0

```

**Analiza wyników:**
Podobnie jak w poprzednim teście, procesy egzaminacyjne przebiegły pomyślnie, a kandydaci uzyskali oceny. Weryfikacji poddano raporty końcowe.

1. **Lista Przyjętych:**
Plik zawiera jedynie nagłówki kolumn, co jest zachowaniem oczekiwanym.

```text
PID | OCENA A | OCENA B | OGOLNA OCENA
(brak rekordów)

```

2. **Lista Rankingowa:**

System zakwalifikował wszystkich zdających (którzy przeszli przez obie komisje) do grupy odrzuconych z powodu braku miejsc.

* Liczba osób przyjętych: **0**
* Liczba osób z wynikiem pozytywnym, ale nieprzyjętych: **796** (wartość zmienna zależna od losowania, w tym przebiegu 796).

```bash
$ cat lista_rankingowa.txt | grep "ZDAL_EGZAMIN_PRZYJETY" | wc -l
0
$ cat lista_rankingowa.txt | grep "ZDAL_EGZAMIN_BRAK_MIEJSC" | wc -l
796

```

**Wniosek B:**
System jest odporny na skrajne konfiguracje (zerowy limit). Algorytm w funkcji `generuj_raporty` poprawnie obsłużył warunek pętli (`licznik_zakwalifikowanych < LIMIT_PRZYJEC`), nie dopuszczając nikogo do grona studentów, mimo wysokich wyników egzaminacyjnych.

## 4.5. Weryfikacja nadmiarowości miejsc (Limit przyjęć > Liczba kandydatów)

### 4.5.1. Cel testu

Sprawdzenie szczelności systemu rekrutacyjnego w sytuacji "rynku kandydata", gdzie liczba dostępnych miejsc przewyższa liczbę osób aplikujących. Test ma na celu udowodnienie, że **limit miejsc nie jest jedynym kryterium przyjęcia**. System musi nadal egzekwować wymogi formalne (zdana matura) oraz merytoryczne (zdane egzaminy wstępne).

**Hipoteza:**
Nawet przy nielimitowanej liczbie miejsc, na listę przyjętych nie trafi żaden kandydat bez matury ani żaden kandydat, który oblał egzaminy w komisjach.

**Konfiguracja środowiska:**
W pliku `common.h` ustawiono limit przyjęć na wartość `1500`, co przy populacji `1000` kandydatów gwarantuje miejsce dla każdego. Zwiększono również prawdopodobieństwo braku matury do 20%, aby uzyskać reprezentatywną grupę odrzuconych formalnie.

```c
#define LICZBA_KANDYDATOW 1000
#define LIMIT_PRZYJEC 1500          // Nadmiar miejsc (150% populacji)
#define SZANSA_NA_BRAK_MATURY 20    // Zwiększone ryzyko odrzucenia formalnego
#define SZANSA_NA_ZDANA_TEORIE 2    // Standardowo

```

### 4.5.2 Analiza wyników

Po przeprowadzeniu symulacji przeanalizowano pliki wynikowe pod kątem spójności danych.

**1. Analiza przyjęć (`ZDAL_EGZAMIN_PRZYJETY`)**
Liczba osób przyjętych wyniosła **653**. Jest to znacznie mniej niż liczba dostępnych miejsc (1500) oraz mniej niż całkowita liczba kandydatów (1000). Oznacza to, że system zadziałał selektywnie.

```bash
$ cat lista_rankingowa.txt | grep "ZDAL_EGZAMIN_PRZYJETY" | wc -l
653

```

**2. Weryfikacja odrzuceń formalnych (`BRAK_MATURY`)**
System poprawnie zidentyfikował i odrzucił osoby bez uprawnień wstępnych. Liczba ta (193) jest statystycznie bliska założonym 20% populacji.

```bash
$ cat lista_rankingowa.txt | grep "BRAK_MATURY" | wc -l
193

```

*Wniosek:* Mimo wolnych miejsc, osoby bez matury nie zostały dopuszczone do procesu rekrutacji.

**3. Weryfikacja odrzuceń merytorycznych (`OBLAL`)**
Osoby, które posiadały maturę, ale nie uzyskały wymaganej liczby punktów w Komisji A lub B, otrzymały status "OBLAL".

```bash
$ cat lista_rankingowa.txt | grep "OBLAL" | wc -l
154

```

*Wniosek:* Wolne miejsca nie gwarantują przyjęcia osobom, które nie reprezentują odpowiedniego poziomu wiedzy.

**4. Weryfikacja statusu "Brak Miejsc" (`ZDAL_EGZAMIN_BRAK_MIEJSC`)**
Zgodnie z oczekiwaniami, przy nadmiarowym limicie przyjęć, kategoria osób, które zdały, ale się nie dostały, powinna być pusta.

```bash
$ cat lista_rankingowa.txt | grep "ZDAL_EGZAMIN_BRAK_MIEJSC" | wc -l
0

```

*Wniosek:* Każdy kandydat spełniający kryteria merytoryczne i formalne został przyjęty. System nie odrzucił nikogo "bez powodu".

**5. Spójność raportów**
Porównano liczbę rekordów w pliku `lista_przyjetych.txt` z liczbą osób o statusie przyjętym w rankingu.
Plik zawiera **654** linie (1 linia nagłówka + 653 rekordy).

```bash
$ cat lista_przyjetych.txt | wc -l
654

```

Liczba ta (`654 - 1 = 653`) idealnie pokrywa się z wynikiem z punktu 1.

### 4.5.3 Wniosek końcowy

Test zakończył się sukcesem. Udowodniono, że algorytm rekrutacyjny w funkcji `generuj_raporty` działa prawidłowo w warunkach nadmiaru miejsc. System zachowuje szczelność – **miejsce na uczelni otrzymują wyłącznie kandydaci, którzy przeszli pełną ścieżkę weryfikacji (Matura -> Komisja A -> Komisja B)**, niezależnie od liczby wolnych wakatów. Nie wystąpiło zjawisko "przyjmowania wszystkich jak leci".

## 5. Problemy napotkane podczas realizacji projektu

Opis głownych problemów (nie wszystkich) które występowały podczas tworzenia projektu.

### 5.1. Wyścig wątków

Wątki egzaminatorów uruchamiały się szybciej, niż Przewodniczący zdążył wpisać dane studenta do struktury stolika, co powodowało błędy odczytu (PID = -1).

* **Rozwiązanie:** Wprowadzono flagę `gotowe`, która wstrzymuje egzaminatorów do momentu pełnego przygotowania danych przez Przewodniczącego.

### 5.2. Zakleszczenia przy synchronizacji

Próby synchronizowania fazy *Pytanie -> Odpowiedź* za pomocą funkcji `sleep()` powodowały, że procesy blokowały się nawzajem, czekając w nieskończoność.

* **Rozwiązanie:** Zastąpiono czasowe oczekiwanie **zmiennymi warunkowymi** (`cond`) i prostą maszyną stanów w pamięci dzielonej, sterowaną sekwencyjnie przez Przewodniczącego.

### 5.3. Procesy Zombie

Przy próbie obsługi całej symulacji w jednym pliku źródłowym, procesy potomne nie były poprawnie "odbierane" przez rodzica, zaśmiecając tablicę procesów systemu.

* **Rozwiązanie:** Rozdzielono projekt na niezależne pliki wykonywalne (`execl`) i zaimplementowano w Dziekanie obsługę sygnału `SIGCHLD` (funkcja `wait`).

### 5.4. Trwałość obiektów IPC po awarii

Semafory i pamięć dzielona nie czyszczą się same po przerwaniu programu (Ctrl+C). Przy ponownym uruchomieniu aplikacja czytała stare wartości, co blokowało start.

* **Rozwiązanie:** Dodano procedurę "twardego resetu" (`sem_unlink`, `shm_unlink`) na samym początku funkcji `main`, gwarantując czysty start symulacji.

### 5.5. Synchronizacja grupowa przy stoliku

Trudność w zapewnieniu, aby wszyscy członkowie komisji zadali pytania w tej samej turze, nie wchodząc sobie w słowo.

* **Rozwiązanie:** Zastosowano **bariery** (`pthread_barrier`), które zatrzymują wątki w kluczowych punktach, wymuszając pracę "ramię w ramię".

### 5.6. Analiza wykorzystywanych struktur (Optymalizacja synchronizacji)

Przez długi czas kwestionowałem poprawność i zasadność umieszczania indywidualnego `mutex_ipc` w strukturze każdego Studenta w pamięci dzielonej. Chciałem uniknąć tworzenia tak wielu obiektów synchronizacyjnych, obawiając się, że przy dużej liczbie kandydatów system zarezerwuje zbyt dużo zasobów pamięci, co negatywnie wpłynie na działanie symulacji.

* **Rozwiązanie:** Po analizie zdecydowałem się na pozostawienie indywidualnych mutexów. Obliczyłem, że koszt ten jest pomijalny – standardowy mutex zajmuje 40 bajtów, co przy 1200 kandydatach daje łącznie jedynie około 47 KB pamięci RAM (wartość nieistotna dla współczesnych komputerów). Co ważniejsze, ten konkretny mutex ma fundamentalne znaczenie dla szybkości działania: blokuje on dostęp wyłącznie do fragmentu pamięci jednego studenta. Dzięki temu wszystkie pozostałe procesy mogą działać w tle bez wzajemnego blokowania się, co nie byłoby możliwe przy jednym głównym mutexie na całą tablicę. Gwarantuje to również, że Komisja nigdy nie nadpisze danych w momencie, w którym kandydat próbuje je odczytać.

---

## 6. Implementacja wymaganych konstrukcji systemowych

Poniżej przedstawiono zestawienie wykorzystanych w projekcie funkcji systemowych (zgodnych ze standardem POSIX), podzielonych na kategorie wymagane w specyfikacji projektowej.

### 6.1 Tworzenie i obsługa plików

Zastosowano niskopoziomowe operacje wejścia/wyjścia do obsługi potoków nazwanych oraz standardowe I/O do obsługi logów i raportów.

* **`open()`** - [[Kandydat] Otwarcie potoku FIFO i wysłanie struktury zgłoszenia przez Kandydata.](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/be18b7eae0fccc711c2f3cb5a19f8a7750a6690e/kandydat.c#L36)
* **`write()`** - [[Kandydat] Wysłanie zgloszenia przez kandydata](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/bf8e8004d1e100064a459f56cd43bbe013f766d9/kandydat.c#L47)

* **`close()`**- [[Dziekan] Zamkniecie fifo](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/bf8e8004d1e100064a459f56cd43bbe013f766d9/dziekan.c#L356)

* **`read()`** - [[Dziekan] Odczyt zgłoszeń z potoku FIFO przez proces Dziekana.](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/bf8e8004d1e100064a459f56cd43bbe013f766d9/dziekan.c#L303)

* **`unlink()`** - [[Dziekan] Usunięcie pliku potoku nazwanego(FIFO) z systemu plików po zakończeniu symulacji.](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/bf8e8004d1e100064a459f56cd43bbe013f766d9/dziekan.c#L357)

* **`fopen()`** - [[Dziekan] Generowanie trwałych plików z wynikami (Listy Rankingowe).](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/bf8e8004d1e100064a459f56cd43bbe013f766d9/dziekan.c#L38)

* **`fprintf()`** – [[Logger] Generowanie trwałych plików z wynikami (Listy Rankingowe)](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/bf8e8004d1e100064a459f56cd43bbe013f766d9/common.h#L134)

* **`fclose()`** [[Dziekan] Generowanie trwałych plików z wynikami (Listy Rankingowe).](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/bf8e8004d1e100064a459f56cd43bbe013f766d9/dziekan.c#L66)

### 6.2. Tworzenie procesów

Zarządzanie cyklem życia procesów realizowane jest przez proces zarządcy (Dziekana).

* **`fork()`** – [[Dziekan] Utworzenie nowego procesu kandydatów.](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/bf8e8004d1e100064a459f56cd43bbe013f766d9/dziekan.c#L267)

* **`execl()`** – [[Dziekan] Nadpisanie obrazu procesu kodem programu potomnego komisji.](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/bf8e8004d1e100064a459f56cd43bbe013f766d9/dziekan.c#L239)

* **`wait()`** –  [[Dziekan] Czekania na zakończenie procesów w kodzie rodzica.](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/bf8e8004d1e100064a459f56cd43bbe013f766d9/dziekan.c#L375)

* **`waitpid()`** - [[Dziekan] Obieranie bez blokowania się zwłok procesów potomnych (sprzatanie zoombie).](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/bf8e8004d1e100064a459f56cd43bbe013f766d9/dziekan.c#L111)

* **`exit()`** - [[Dziekan] Kontrolowane zakończenie procesu w przypadku błędu `execl` lub sygnału.](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/0c3e33182ebb1867013141757f2b2502aef94ee2/dziekan.c#L240)

### 6.3. Tworzenie i obsługa wątków

Wielowątkowość wykorzystano w procesach Komisji do równoległej obsługi wielu stanowisk egzaminacyjnych.

* **`pthread_create()`** - [[Komisja] Utworzenie puli wątków egzaminatorów](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/0c3e33182ebb1867013141757f2b2502aef94ee2/komisja.c#L600)
  
* **`pthread_join()`** - [[Komisja] Oczekiwanie na wykonanie zadań przez pule wątków egzaminatorów](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/0c3e33182ebb1867013141757f2b2502aef94ee2/komisja.c#L609)

* **`pthread_mutex_lock()`**, **`pthread_mutex_unlock()`** –  [[Komisja] Ochrona sekcji krytycznych (dostęp do danych stolika i rekrutacja wątków).](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/0c3e33182ebb1867013141757f2b2502aef94ee2/komisja.c#L37-L40)

* **`pthread_cond_wait()`** – [[Komisja] Informacja dla watków aby zasnał i czekał na sygnał egzaminujacego (zabiera CPU)](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/0c3e33182ebb1867013141757f2b2502aef94ee2/komisja.c#L125)

* **`pthread_cond_signal()`** – [[Komisja] Sygnał dla procesu kandydata informujacy o wyłanych pytaniach przez komisje](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/0c3e33182ebb1867013141757f2b2502aef94ee2/komisja.c#L260)

* **`pthread_cond_broadcast()`**
[[Komisja] Obudzenie człnków do pracy przy stoliku *powiazane z pthread_cond_wait()*](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/0c3e33182ebb1867013141757f2b2502aef94ee2/komisja.c#L233)

* **`pthread_cond_timedwait()`** – [[Komisja] Oczekiwanie na odpowiedź kandydata z mechanizmem timeoutu](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/0c3e33182ebb1867013141757f2b2502aef94ee2/komisja.c#L270)

* **`pthread_barrier_wait()`** – [[Komisja] Synchronizacja grupowa, zbiórka członków komisji przy stoliku](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/0c3e33182ebb1867013141757f2b2502aef94ee2/komisja.c#L237)

### 6.4. Obsługa sygnałów

System reaguje na sygnały systemowe w celu zapewnienia stabilności i obsługi ewakuacji.

* **`sigaction()`** – [[Dziekan] Rejestracja zaawansowanych handlerów dla `SIGCHLD`.](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/0c3e33182ebb1867013141757f2b2502aef94ee2/dziekan.c#L142C5-L142C14)

* **`kill()`** – [[Dziekan] Wysłanie sygnału kończącego (`SIGTERM`) do grupy procesów komisji.](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/0c3e33182ebb1867013141757f2b2502aef94ee2/dziekan.c#L371)

### 6.5. Synchronizacja procesów (Semafory POSIX)

Do sterowania przepływem kandydatów między instancjami (Dziekan -> A -> B) wykorzystano semafory nazwane.

* **`sem_open()`**, **`sem_close()`** - [[Dziekan] Tworzenie semaforów z flaga O_CREATE oraz zamykanie nieuzywanych w procesie](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/0c3e33182ebb1867013141757f2b2502aef94ee2/dziekan.c#L213-L226)

* **`sem_unlink()`** - [[Dziekan] Czyszczenie zasobów przed uruchomieniem symulacji.](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/0c3e33182ebb1867013141757f2b2502aef94ee2/dziekan.c#L153-L159)

* **`sem_wait()`** -[[Komisja] Blokowanie procesu Komisji w oczekiwaniu na kandydata.](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/0c3e33182ebb1867013141757f2b2502aef94ee2/komisja.c#L172)

* **`sem_post()`** – [[Dziekan] Sygnalizowanie Komisji dostępności kandydata przez Dziekana.](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/0c3e33182ebb1867013141757f2b2502aef94ee2/dziekan.c#L331-L337)

### 6.6. Łącza nazwane (FIFO)

Kanał komunikacyjny służący do przesyłania struktur zgłoszeniowych od Kandydatów do Dziekana.

* **`mkfifo()`** – [[Dziekan] Utworzenie pliku specjalnego kolejki FIFO z minimalnymi prawami.](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/0c3e33182ebb1867013141757f2b2502aef94ee2/dziekan.c#L166)

### 6.7. Segmenty pamięci dzielonej (POSIX SHM)

Główny magazyn danych (tablica studentów) współdzielony między wszystkimi procesami symulacji.

* **`shm_open()`**, **`ftruncate()`**, **`mmap()`** - [[Dziekan] Utworzenie, nadanie rozmiaru i zmapowanie obiektu pamięci.](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/0c3e33182ebb1867013141757f2b2502aef94ee2/dziekan.c#L176-L183)

* **`shm_unlink()`** - [[Dziekan] Usunięcie obiektu pamięci z systemu po zakończeniu symulacji.](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/0c3e33182ebb1867013141757f2b2502aef94ee2/dziekan.c#L167)

* **Atrybut `PTHREAD_PROCESS_SHARED`** - [[Dziekan] Konfiguracja mutexów i zmiennych warunkowych do pracy w pamięci dzielonej (między procesami](https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/blob/0c3e33182ebb1867013141757f2b2502aef94ee2/dziekan.c#L194-L197)
## 7. Podsumowanie

### 7.1. Wyzwania projektowe

Projekt wymagał głebokiego zastanowienia i analizy treści zadania. Jeszcze trudniejsza okazała się implemetnacja struktur i implementacja wszytskich procesów oraz wątków wykorzystywanych do realizacji symulacji. Największym wyzwaniem okazało się zaprojektowanie struktury pamięci dzielonej w taki sposób, aby była czytelna i stosunkowo optymalna pod kątem wydajności. Zależało mi, aby symulacja działała możliwie jak najwydajniej, bez polegania na sztywnych ograniczeniach czasowych (np. sleep) jako metodzie synchronizacji.

### 7.2. Optymalizacja struktury danych i zarządzania zasobami

Długo zastanawiałem się, jak zorganizować tablice studentów i flagi statusów, aby uniknąć zbędnego kopiowania danych, a jednocześnie umożliwić procesom szybki dostęp. Kiedy w końcu udało mi się ułożyć ten schemat w myślach i przelać go na kod, wyzwaniem stało się zarządzanie procesami oraz wątkami tak, aby podczas oczekiwania na zdarzenia nie wykorzystywały niepotrzebnie zasobów procesora.

### 7.3. Wynik końcowy

Po wielu dniach pracy udało mi się stworzyć symulację, która działa płynnie i nie zakleszcza się w żadnym nieoczekiwanym momencie.
