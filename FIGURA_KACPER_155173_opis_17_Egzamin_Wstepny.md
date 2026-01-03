Temat 17 – Egzamin wstępny.
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

Github: https://github.com/Kazurek11/Egzamin-Wstepny-Systemy-Operacyjne/

### Aktualizacje prac 28.12.2025

#### Commit 3

W poprzednich 2 commitach nie dodawałem opisu, ponieważ dotyczyły one podstawowej struktury (FIFO, struktura kandydata), co było mniej złożone niż obecna implementacja synchronizacji.

Względem poprzedniego commita dodałem mechanizmy synchronizacji międzyprocesowej:

* **Semafory nazwane:** Utworzyłem w procesie Dziekana semafory kontrolujące kolejkę oraz liczbę wolnych miejsc w salach. Procesy Komisji otwierają te semafory (`sem_open`), co umożliwia sterowanie przepływem studentów.
* **Pamięć Dzielona:** Zaimplementowałem mapowanie pamięci dzielonej w procesie Komisji, aby umożliwić odczyt i zapis danych studentów współdzielonych z Dziekanem.
* **Mutexy:** W kodzie Komisji dodałem mutexy, które posłużą do synchronizacji pracy wątków (egzaminatorów) wewnątrz jednej sali, zapobiegając wyścigom danych (*race conditions*).

#### Jaki jest plan na następne commity

Pozostaję przy jednym pliku `komisja.c`, który będzie obsługiwał logikę obu komisji zależnie od argumentu uruchomienia ('A' lub 'B'). Planuję zaimplementować czesciowa logikę egzaminu (do momentu odpowiedzi kandyata):

1.  **Wejście:** Przewodniczący będzie oczekiwał na semaforze kolejki (sygnał od Dziekana) i zajmował miejsce w sali (`sem_wait` na semaforze miejsc).
2.  **Identyfikacja:** Pobranie `ID_studenta` z Pamięci Dzielonej do zmiennej globalnej w procesie komisji.
3.  **Egzamin:** Zastosuję zmienną globalną `ilosc_zadanych_pytan` chronioną mutexem. Każdy z wątków (członkowie + przewodniczący) zada pytanie i zwiększy licznik.
4.  **Synchronizacja:** Gdy licznik osiągnie wymaganą wartość (np. 5 dla Komisji A), ostatni wątek wyśle sygnał `pthread_cond_signal`, co pozwoli Przewodniczącemu przejść do fazy oceniania i odesłania kandydata dalej.

### Aktualizacja prac (29.12.2025)

### Implementacja logiki Komisji i synchronizacja wątków

**Zrealizowane funkcjonalności:**

1.  **Synchronizacja Procesów (Dziekan <-> Komisja):**
    * Zaimplementowano mechanizm startowy: Dziekan zwiększa semafor odpowiedniej kolejki (A/B), co budzi Przewodniczącego komisji.
    * Komisja zarządza przepustowością sali poprzez semafor liczby wolnych miejsc.
    * **Fix:** Dziekan na starcie czyści stare semafory (`sem_unlink`), co rozwiązuje problem "widmowych kandydatów" (przetwarzania ID -1) po restarcie programu.

2.  **Struktura i Logika Stanowiska:**
    * Zaimplementowano strukturę `Stanowisko` przypisaną do każdego fizycznego miejsca w sali, co pozwala na równoległą obsługę wielu kandydatów.
    * **Unikalność pytań:** Wprowadzono tablicę `kto_pytal`, która gwarantuje, że każdy z egzaminatorów (wątków) zada dokładnie jedno, unikalne pytanie danemu kandydatowi.
    * **Flagi synchronizacji:** Dodano pole `gotowe` (zapobiega wyścigom wątków przed wpisaniem danych kandydata) oraz `zajete`.
    * **Ocenianie:** Dodano zmienną `suma_ocen`, która agreguje punkty od wszystkich członków komisji w celu wyliczenia średniej przez Przewodniczącego.

3.  **Konfiguracja:**
    * Zdefiniowano zmienne czasowe `GODZINA_T` oraz `GODZINA_Ti` do symulacji upływu czasu.

### Plan na następny commit:

* **Interakcja z Kandydatem:** Implementacja logiki oczekiwania kandydata na komplet pytań przed udzieleniem odpowiedz.
* **Dwukierunkowa komunikacja:** Opracowanie mechanizmu, w którym Kandydat jest "świadomy" treści pytania i odsyła odpowiedź do konkretnego egzaminatora.
* **Nowa struktura danych:** Stworzenie struktury odpowiedzialnej za wymianę danych na linii: Egzaminator (Pytanie) -> Kandydat (Odpowiedź) -> Egzaminator (Ocena).

### Napotkane problemy i ewolucja rozwiązania (Proces myślowy)

Podczas implementacji natrafiłem na kilka istotnych wyzwań związanych ze współbieżnością, które wymusiły zmianę mojego podejścia do synchronizacji:

1.  **Wyścig wątków ("Kandydat -1"**)
    Moim głównym problemem było to, że wątki członków komisji okazywały się "zbyt szybkie". Po obudzeniu semaforem potrafiły wejść do sekcji krytycznej i próbować egzaminować kandydata, zanim Przewodniczący zdążył w ogóle przepisać jego dane z pamięci dzielonej (SHM) do struktury stolika. Skutkowało to logami z `PID: -1`.
    * **Rozwiązanie:** Zrozumiałem, że sam Mutex to za mało. Wprowadziłem więc dodatkową flagę `gotowe` w strukturze `Stanowisko`. Działa ona jak "bramka" – Przewodniczący otwiera ją (ustawia na 1) dopiero w momencie, gdy fizycznie posadzi kandydata i uzupełni jego papiery. Dopiero wtedy reszta wątków może ruszyć do pracy.

2.  **Trwałość semaforów**
    Zauważyłem, że gdy przerywam program (Ctrl+C), semafory systemowe nie zerują się same. Przy kolejnym uruchomieniu aplikacja myślała, że w kolejce wciąż czekają ludzie (stare wartości semafora), podczas gdy pamięć dzielona była pusta. To powodowało chaos na starcie.
    * **Rozwiązanie:** Dodałem "twardy reset" (`sem_unlink`) na samym początku kodu Dziekana. Teraz każde uruchomienie programu gwarantuje start z czystą kartą, niezależnie od tego, jak zakończyło się poprzednie.

3.  **Logistyka pytań (Kto już pytał?):**
    Musiałem też wymyślić sposób, aby zagwarantować, że przy jednym studencie każdy egzaminator zada *dokładnie jedno* pytanie. Bez tego, szybsze wątki mogłyby "ukraść" pytania wolniejszym i zdominować egzamin.
    * **Rozwiązanie:** Zastosowałem tablicę `kto_pytal` przy każdym stanowisku. Działa to jak lista obecności przy konkretnym stole – wątek sprawdza, czy już tu pytał. Jeśli tak, ustępuje miejsca innym. Dopiero gdy licznik pytań osiągnie `MAX - 1`, do gry wkracza Przewodniczący z ostatnim, decydującym pytaniem.
  
### Aktualizacja prac (Commit 4): Logika Kandydata i pełna synchronizacja egzaminu

W tym etapie skupiłem się na domknięciu pętli komunikacyjnej. Proces `Kandydat` przestał być tylko "generatorem zgłoszeń" do kolejki FIFO, a stał się aktywnym uczestnikiem egzaminu, który reaguje na polecenia Komisji i udziela odpowiedzi w czasie rzeczywistym.

**Zrealizowane zmiany:**

1.  **Aktywny proces Kandydata (`kandydat.c`)**
    Zmodyfikowałem kod tak, aby proces nie kończył działania zaraz po wysłaniu PID. Teraz:
    * Mapuje pamięć dzieloną (SHM).
    * Odnajduje swój rekord w tablicy studentów.
    * Aktywnie oczekuje na nadejście pytań od komisji.
    * Symuluje "myślenie" (`usleep`) i wpisuje odpowiedzi do struktury.

2.  **Protokół komunikacji (Maszyna Stanów)**
    Zamiast komplikować kod kolejnymi mutexami międzyprocesowymi, oparłem synchronizację na prostej fladze stanu `status_arkusza` w pamięci dzielonej. Działa to jak prosta maszyna stanów:
    * **Stan `0` (Zadawanie):** Komisja wpisuje pytania, kandydat czeka.
    * **Stan `1` (Odpowiadanie):** Przewodniczący zmienia flagę. Komisja czeka, Kandydat przetwarza pytania i wpisuje odpowiedzi.
    * **Stan `2` (Ocenianie):** Kandydat kończy pisać, Komisja sprawdza i wystawia oceny.

3.  **Logika dla osób powtarzających (Retakers)**
    Doprecyzowałem przepływ dla studentów, którzy mają już zaliczoną teorię.
    * Trafiają oni normalnie do sali Komisji A.
    * Przewodniczący wykrywa flagę `zdana_teoria`.
    * Faza zadawania pytań jest **pomijana**.
    * Student jest natychmiast przekierowywany do kolejki przed Komisją B.

4.  **Fix: Problem "Widmowego Kandydata" (ID -1)**
    Rozwiązałem krytyczny błąd wyścigu (race condition), gdzie wątki egzaminatorów budziły się szybciej, niż Przewodniczący zdążył wpisać dane studenta do struktury stolika.
    * **Rozwiązanie:** Dodałem walidację indeksów oraz "twardy reset" semaforów (`sem_unlink`) przy starcie procesu Dziekana. Dzięki temu restart aplikacji nie powoduje już błędów segmentacji na start.

**Napotkane problemy i wnioski:**
Największym wyzwaniem było zsynchronizowanie momentu przejścia z zadawania pytań do odpowiadania. Początkowo procesy blokowały się nawzajem (deadlock). Rozwiązałem to, czyniąc Przewodniczącego "koordynatorem" sali – to on, po upewnieniu się, że wszyscy członkowie komisji zadali pytania, zmienia stan flagi, dając sygnał procesowi kandydata do rozpoczęcia pracy.

### Aktualizacja Postępów – 03.01.2026 "Poprawienie logiki oraz testowanie wydajnosci symulacji w systemie"

**1. Logika biznesowa i raportowanie**

* **Poprawa logiki i logów:** Uszczelniłem logikę przepływu kandydatów oraz formatowanie logów. Każdy wpis w logach zawiera teraz precyzyjne identyfikatory **PID** procesu oraz **TID** wątku, co znacznie ułatwia debugowanie.
* **Listy rankingowe:** Proces *Dziekan* poprawnie generuje finalną listę rankingową oraz listę osób przyjętych.
* **Algorytm selekcji:** Zaimplementowałem własny algorytm sortowania, który odpowiada za selekcję kandydatów na podstawie wyników.
* **System oceniania:** Zgodnie z wytycznymi, ocena końcowa jest średnią arytmetyczną z obu komisji: `(Ocena A + Ocena B) / 2`.
* **Współczynnik przyjęć:** System trzyma się założenia logicznego, w którym średnio co 10. kandydat otrzymuje miejsce na roku.

**2. Wydajność i zarządzanie zasobami (WSL)**

* Przeprowadziłem analizę procesów (aktywne, śpiące, zombie) przy użyciu narzędzia `htop`.
* Mimo że samo środowisko WSL (Windows Subsystem for Linux) rezerwuje sporo zasobów, sama symulacja jest bardzo lekka i zużywa jedynie **ok. 2% mocy procesora**.
* **Brak procesów zombie:** Przy obecnej architekturze ("rozbite" pliki binarne uruchamiane przez `execl`) procesy są poprawnie sprzątane i nie tworzą się tzw. *zombie processes*.

**3. Napotkane problemy i rozwiązania**

* Podjąłem próbę przeniesienia całej symulacji do jednego pliku `main.c` (odpowiedzialnego za inicjalizację IPC i tworzenie procesów potomnych). Niestety, skutkowało to masowym powstawaniem procesów zombie (liczba zombie równała się liczbie kandydatów).
* **Decyzja:** Pozostałem przy pierwotnej, sprawdzonej implementacji z podziałem na osobne pliki wykonywalne (`dziekan`, `komisja`, `kandydat`), która działa stabilnie i poprawnie zarządza cyklem życia procesów.

** Plany na kolejne kroki:**
* Utworzenie dokumentacji technicznej.
* Uporządkowanie kodu (linkowanie funkcji/nagłówki).
* Implementacja przekierowania logów do osobnych plików tekstowych dla Dziekana, Kandydata i Komisji (zamiast wspólnego wyjścia na `stdout`).