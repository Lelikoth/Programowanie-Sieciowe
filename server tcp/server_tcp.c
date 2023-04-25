//Komunikacja pomiędzy klientem a serwerem odbywa się przy pomocy połączenia strumieniowego. Klient wysyła jedną lub więcej linii zawierających wyrażenia. Dla każdej odebranej linii serwer zwraca linię zawierającą pojedynczą liczbę (obliczony wynik) albo komunikat o błędzie.
//Ogólna definicja linii jest zapożyczona z innych protokołów tekstowych: ciąg drukowalnych znaków ASCII (być może pusty) zakończony dwuznakiem \r\n.
//Linia z wyrażeniem może zawierać tylko cyfry oraz znaki plusa i minusa. Postać wyrażeń jest dokładnie taka, jak w poprzednio rozważanym protokole datagramowym (a więc wyrażenie musi zawierać przynajmniej jedną liczbę itd.).
//Linia z odpowiedzią serwera zawiera albo opcjonalny znak minusa i niepusty ciąg cyfr, albo pięć liter składających się na słowo „ERROR”.
//Wszystkie linie, i te wysyłane przez klientów, i przez serwer, mają oczywiście do opisanej powyżej zawartości dołączony terminator linii, czyli \r\n.
//Serwer może, ale nie musi, zamykać połączenie w reakcji na nienaturalne zachowanie klienta. Obejmuje to wysyłanie danych binarnych zamiast znaków ASCII, wysyłanie linii o długości przekraczającej przyjęty w kodzie źródłowym serwera limit, długi okres nieaktywności klienta, itd. Jeśli serwer narzuca maksymalną długość linii, to limit ten powinien wynosić co najmniej 1024 bajty (1022 drukowalne znaki i dwubajtowy terminator linii).
//Serwer nie powinien zamykać połączenia gdy udało mu się odebrać poprawną linię w sensie ogólnej definicji, ale dane w niej zawarte są niepoprawne (np. oprócz cyfr i operatorów arytmetycznych są przecinki). Powinien wtedy zwracać komunikat o błędzie i przechodzić do przetwarzania następnej linii przesłanej przez klienta.
//Serwer powinien zwracać komunikat błędu gdy przesłane przez klienta liczby bądź wyniki prowadzonych na nich obliczeń przekraczają zakres typu całkowitoliczbowego wykorzystywanego przez serwer.

#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdint.h>
#include <limits.h>
#include <pthread.h>

#define MaxData 1024

// klient handler
void *client_handler(void *arg)
{
    // gniazdko
    int gniazdko = *(int *)arg;
    // zwolnienie pamięci
    free(arg);

    // bufor
    char bufor[MaxData];
    char res[MaxData];
    bool koniec_zapytania = false;
    bool puste_zapytanie = true;

    // liczba i wynik
    int16_t wynik = 0;
    int16_t liczba = 0;
    
    // operator
    char operator= '+';
    bool znak = true;
    
    bool blad = false;

    // odbieranie danych
    while (1)
    {
        // zerowanie bufora
        memset(bufor, 0, sizeof bufor);
        ssize_t bajty; 
        
        // odbieranie danych
        if( (bajty = recv(gniazdko, bufor, sizeof(bufor), 0)) == -1 )
        {
            printf("recv");
            pthread_exit(NULL);
        }
        else
        {
            printf("Received : %s\n", bufor);
        }

        // sprawdzanie czy zapytanie jest puste
        if (bajty == 0)
        {
            if (puste_zapytanie)
            {
                printf("puste zapytanie\n");
                break;
            }
            else
            {
                printf("koniec zapytania\n");
                koniec_zapytania = true;
            }
        }
        else
        {
            puste_zapytanie = false;
        }

        // przetwarzanie danych
        for (int i = 0; i < bajty; i++)
        {
            printf("%d , %d , %d , %d \n", wynik, liczba, bufor[i], koniec_zapytania);

            // sprawdzanie czy znak jest liczbą
            if (bufor[i] == '\n' && koniec_zapytania)
            {   
                //dodawanie
                if (operator== '+')
                {
                    if (wynik <= 0 || (INT16_MAX - wynik >= liczba))
                    {
                        wynik += liczba;
                        liczba = 0;
                    }
                    else
                    {
                        blad = true;
                    }
                }
                //odejmowanie
                else if (operator== '-')
                {
                    if (wynik >= 0 || (wynik - INT16_MIN >= liczba))
                    {
                        wynik -= liczba;
                        liczba = 0;
                    }
                    else
                    {
                        blad = true;
                    }
                }
                //blad
                if (blad)
                {
                    //wysylanie bledu
                    memcpy(res, "ERROR\r\n", 7);
                    int resbajty = send(gniazdko, res, 7, 0);
                    if (resbajty == -1)
                    {
                        printf("send");
                        pthread_exit(NULL);
                    }
                    printf("ERROR\n");
                }
                else
                {
                    //wysylanie wyniku
                    char wynikliczba[20];

                    sprintf(wynikliczba, "%d\r\n", wynik);
                    
                    memcpy(res, wynikliczba, sizeof(wynikliczba));
                    
                    int resbajty;
                    
                    if( ( resbajty = send(gniazdko, res, strlen(wynikliczba), 0)) == -1)
                    {
                        printf("send");
                        pthread_exit(NULL);
                    }
                    printf("SENT = %s\n", wynikliczba);
                }
                //resetowanie zmiennych
                koniec_zapytania = false;
                
                blad = false;
                
                znak = true;
                
                wynik = 0;
                liczba = 0;
            }
            //sprawdzanie czy znak jest znakiem nowej linii
            else if (bufor[i] == '\r')
            {
                if (znak)
                {
                    blad = true;
                }
                koniec_zapytania = true;
            }
            //sprawdzanie czy znak to cyfra
            else if (bufor[i] >= '0' && bufor[i] <= '9')
            {
                //sprawdzanie czy liczba nie przekracza zakresu
                if (liczba == 0)
                {
                    liczba = bufor[i] - '0';
                }
                else if (INT16_MAX / (liczba * 10 + (bufor[i] - '0')) >= 1)
                {
                    liczba = liczba * 10 + (bufor[i] - '0');
                }
                else
                {
                    blad = true;
                }
                znak = false;
            }
            //sprawdzanie czy znak to operator
            else if ((bufor[i] == '-' || bufor[i] == '+') && !znak)
            {
                //sprawdzanie czy operator nie jest pierwszym znakiem
                if (bufor[i - 1] == '+' || bufor[i - 1] == '-')
                {
                    blad = true;
                }
                //dodawanie
                if (operator== '+')
                {
                    if (wynik <= 0 || (INT16_MAX - wynik >= liczba))
                    {
                        wynik += liczba;
                        liczba = 0;
                    }
                    else
                    {
                        blad = true;
                    }
                }
                //odejmowanie
                else if (operator== '-')
                {
                    if (wynik >= 0 || (wynik - INT16_MIN >= liczba))
                    {
                        wynik -= liczba;
                        liczba = 0;
                    }
                    else
                    {
                        blad = true;
                    }
                }
                else
                {
                    blad = true;
                }
                //przypisanie operatora
                operator= bufor[i];

                znak = true;
            }
            else
            {
                blad = true;
            }
        }
    }

    //zamykanie gniazdka
    close(gniazdko);
    pthread_exit(NULL);
}

int main()
{
    //tworzenie gniazdka
    int port = 2020;
    int gniazdko_sluch;
    int gniazdko;
    int rcbajty;

    if((gniazdko_sluch = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("socket");
        exit(1);
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr = {.s_addr = htonl(INADDR_ANY)},
        .sin_port = htons(port)};

    if((rcbajty = bind(gniazdko_sluch, (struct sockaddr *)&addr, sizeof(addr))) == -1)
    {
        printf("bind");
        exit(1);
    }

    if ((rcbajty = listen(gniazdko_sluch, 10)) == -1)
    {
        printf("listen");
        exit(-1);
    }

    //przyjmowanie polaczen
    while (1)
    {
        //przyjmowanie polaczenia
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        if (( gniazdko = accept(gniazdko_sluch, (struct sockaddr *)&client_addr, &client_addr_len )) == -1)
        {
            printf("accept");
            continue;
        }

        pthread_t thread_id;
        //przekazywanie gniazdka do watku
        int *p_gniazdko = malloc(sizeof(int));
        *p_gniazdko = gniazdko;
        //tworzenie watku
        if ((rcbajty = pthread_create(&thread_id, NULL, client_handler, p_gniazdko)) != 0)
        {
            printf("pthread_create");
            free(p_gniazdko);
            close(gniazdko);
            continue;
        }
        //odlaczenie watku
        if ((rcbajty = pthread_detach(thread_id)) != 0)
        {
            printf("pthread_detach");
            free(p_gniazdko);
            close(gniazdko);
            continue;
        }
    }
}