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

### Aktualizacje prac

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

