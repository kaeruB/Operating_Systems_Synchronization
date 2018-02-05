# Operating_Systems_Synchronization
Readers/Writers problem - threads synchronisation using mutexes and semaphores


Opis problemu
Jednym z najczęściej spotykanych problemów synchronizacji jest "Problem czytelników i pisarzy". Grupa czytelników czyta dane z określonego miejsca (np. rekordu w bazie danych) a pisarze go zapisują. Ten sam obiekt może czytać więcej niż jeden czytelnik. W przeciwieństwie do nich, pisarze muszą posiadać wyłączny dostęp do obiektu dzielonego. Równocześnie z pisarzem dostępu do zasobu nie może otrzymać ani inny pisarz, ani czytelnik, gdyż mogłoby to spowodować błędy.
Problem ten posiada kilka wariantów:
Wariant faworyzujący czytelników
Czytelnicy nie mają obowiązku czekania na otrzymanie dostępu do zasobu, jeśli w danym momencie nie otrzymał go pisarz. Ponieważ pisarz może otrzymać tylko dostęp wyłączny, musi czekać na opuszczenie zasobu przez wszystkie inne procesy. Jeśli czytelnicy przybywają odpowiednio szybko, może dojść do zagłodzenia pisarza: w tej sytuacji będzie on w nieskończoność czekał na zwolnienie zasobu przez wciąż napływających nowych czytelników.
Wariant faworyzujący pisarzy
Czytelnicy nie mogą otrzymać dostępu do zasobu, jeżeli oczekuje na niego pisarz. W tej sytuacji oczekujący pisarz otrzymuje dostęp najwcześniej, jak to jest możliwe, czyli zaraz po opuszczeniu zasobu przez ostatni proces, który przybył przed nim. W tym wariancie może dojść do zagłodzenia oczekujących czytelników.
Wariant z kolejką FIFO lub z priorytetami, minimalizujący ryzyko zagłodzenia.
Zadanie
Zaimplementuj wielowątkowy program realizujący poprawne rozwiązanie problemu czytelników i pisarzy, spełniające następujące założenia:

jest wiele wątków czytelników i wiele wątków pisarzy
dane umieszczone w pamięci wspólnej, jest to tablica liczb całkowitych
pisarz cyklicznie modyfikuje w sposób losowy wybrane liczby w tablicy (losuje ilość liczb do modyfikacji, ich pozycje w tablicy oraz wartości tych liczb)
czytelnik uruchamiany jest z jednym argumentem - dzielnik i znajduje w tablicy wszystkie liczby, które się przez niego dzielą bez reszty, wykonując cyklicznie operację przeszukiwania tablicy
pisarz raportuje na standardowym wyjściu wykonanie swojej operacji (modyfikacji tablicy), w wersji opisowej programu (opcja -i) wypisuje indeks i wpisaną wartość
analogiczny raport wykonuje czytelnik, dodając dodatkowo wynik operacji: ilość znalezionych liczb spełniających warunek, a w wersji opisowej programu (opcja -i) podaje indeksy i wartości znalezionych liczb.
Należy wykonać dwie wersje rozwiązania.

a) rozwiązanie wykorzystujące do synchronizacji semafory nienazwane standardu POSIX (zgodne z typem rozwiązań problemu współbieżnego stosującego semafory)

b) rozwiązanie wykorzystujące do synchronizacji muteks i zmienne warunkowe (zgodne z typem rozwiązań problemu współbieżnego stosującego monitor)

Dla każdej z wersji wybierz inny wariant rozwiązania, jeden z możliwością zagłodzenia czytelnika albo pisarza, a drugi np z kolejką FIFO, minimalizującymi to ryzyko.

UWAGA! Należy uwzględnić możliwość jednoczesnego uruchamiania wielu wątków pisarzy.
