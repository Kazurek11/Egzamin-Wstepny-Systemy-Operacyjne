- testowanie zabicia procesu kandydata podczas symulacji
  oczekiwany wynik: Kandydat dojdzie do komisji a tam na odpowiedz na pytania ma max 2s. Po tym czasie kandydat zmienia status w Student w środku pamieci dzielonej na 3 (koniec).
- testowanie zachowania programu kiedy 100%/99% nie zda matury.
  oczekiwany wynik: W momencie gdy `SZANSA_NA_ZDANA_TEORIE == 0` to zaden z uczestników nie powinien zdawać mature.
- testowanie zachowania programu w momencie gdy wszyscy uczestnicy będą mieli zdana czesc teoretyczna.
  oczekiwany wynik: Kazdy ze studentów MUSI miec zdana mature, kazdy student pomimo zdania teorii musi odwiedzić komisje A. Kazdy student musi mieć w logach ocene >30% w komisji A.  
- testowanie zachowania programu kiedy proces komisji A lub B zostanie zabity (w sposób nienaturalny)
  oczekiwany wynik: wykonanie funkcji `handler_smierci_dziecka` i zabicie wszystkich procesów.
- testowanie zachowania programu w momencie gdy  MAX_KANDYDATOW < LICZBA_KANDYDATOW.
  oczekiwany wynik: narazie jest deadlock (pytanie czy musze to jakos rozwiazać inaczej)