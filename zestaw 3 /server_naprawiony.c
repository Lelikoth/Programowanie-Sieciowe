//Napisz serwer UDP/IPv4 nasłuchujący na porcie nr 2020 i implementujący powyższy protokół.
//Serwer musi weryfikować odebrane dane i zwracać komunikat o błędzie jeśli są one nieprawidłowe w sensie zgodności ze specyfikacją protokołu.
//W kodzie używaj zmiennych roboczych któregoś ze standardowych typów całkowitoliczbowych (int, long int, int32_t, itd.).
//Co za tym idzie, odebrany ciąg cyfr będzie mógł reprezentować liczbę zbyt dużą, aby dało się ją zapisać w zmiennej wybranego typu. 
//Może też się zdarzyć, że podczas dodawania bądź odejmowania wystąpi przepełnienie (ang. integer overflow / wraparound). 
//Serwer ma obowiązek wykrywać takie sytuacje i zwracać błąd. Uwadze Państwa polecam pliki nagłówkowe limits.h oraz stdint.h, 
//w których znaleźć można m.in. parę stałych INT_MIN i INT_MAX oraz parę INT32_MIN i INT32_MAX.

// Szkielet serwera TCP/IPv4.
//
// Po podmienieniu SOCK_STREAM na SOCK_DGRAM staje się on szkieletem serwera
// UDP/IPv4 korzystającego z gniazdek działających w trybie połączeniowym.

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

#define MAX_DATA 65527

int gniazdko = -1;

void zamknij_gniazdko(){
    if(gniazdko != -1){
        close(gniazdko);
    }
}

int main()
{
    atexit(zamknij_gniazdko); // zamykanie gniazda przy wyjściu z programu lub błędzie

    ssize_t bajty; // liczba odebranych bajtów

    gniazdko = socket(AF_INET, SOCK_DGRAM, 0); // utworzenie gniazda
    if (gniazdko == -1) {
        perror("Nie udało się utworzyć gniazda");
        exit(1);
    }

    struct sockaddr_in addr = { // adres gniazda
        .sin_family = AF_INET,
        .sin_port = htons(2020),
        .sin_addr.s_addr = htonl(INADDR_ANY)
    };

    if( (bind(gniazdko, (struct sockaddr *) & addr, sizeof(addr))) == -1) { // przypisanie adresu do gniazda
        perror("Nie udało się przypisać adresu do gniazda");
        exit(1);
    }

    while (1) {

        unsigned char buf[MAX_DATA+1]; // bufor na dane
        int16_t wynik = 0; // wynik
        int16_t liczba = 0; // liczba
        bool error; // czy wystąpił błąd
        char operator = '+'; // działanie

        struct sockaddr_in client_addr; // adres klienta
        socklen_t client_addr_len = sizeof(client_addr); // rozmiar struktury

        bajty = recvfrom(gniazdko, buf, sizeof(buf), 0, (struct sockaddr *) &client_addr, &client_addr_len); // odebranie danych
        if (bajty == -1) {
            perror("Nie udało się odebrać danych");
            exit(1);
        }

        if(bajty >= 2) // jeśli jest więcej niż 2 bajty
        {
            if(buf[bajty-1] == '\n' && buf[bajty-2] == '\r') // jeśli ostatnie dwa bajty to \r\n
            {
                bajty -=2;
            }
        }
        else if(bajty >= 1) // jeśli jest więcej niż 1 bajt
        {
            if(buf[bajty-1] == '\n') // jeśli ostatni bajt to
            {
                bajty -=1;
            }
        }

        if(bajty == 0)
        {
            error = true;
        }
        else
        {
            error = false;
        }

        for(int i = 0 ; i < bajty ; i++) // obliczanie wyniku
        {

            if(buf[i] >='0' && buf[i] <= '9') // jeśli jest cyfra
            {
                if(liczba == 0) // jeśli liczba jest zerem
                {
                    liczba = buf[i] - '0';
                }
                else if(INT16_MAX / (liczba * 10 + (buf[i] - '0')) >= 1) // sprawdzanie czy nie przekroczymy zakresu
                {
                    liczba = liczba * 10 + (buf[i] - '0');
                }
                else
                {
                    error = true;
                    break;
                }
            }
            else if((buf[i] == '-' || buf[i] == '+' ) && i != 0) // jeśli jest operator
            {
                if(buf[i-1] == '+' || buf[i-1] == '-') // jeśli poprzedni znak był operatorem
                {
                    error = true;
                    break;
                }
                if(operator == '+') // wykonanie działania
                {
                    if(wynik <= 0 || (INT16_MAX - wynik >= liczba)) // sprawdzanie czy nie przekroczymy zakresu
                    {
                        wynik += liczba;
                        liczba = 0;
                    }
                    else
                    {
                        error = true;
                        break;
                    }
                }
                else if(operator == '-') // wykonanie działania
                {
                    if(wynik >= 0 || (wynik - INT16_MIN >= liczba)) // sprawdzanie czy nie przekroczymy zakresu
                    {
                        wynik -= liczba;
                        liczba = 0;
                    }
                    else
                    {
                        error = true;
                        break;
                    }
                }
                else // jeśli operator jest niepoprawny
                {
                    error = true;
                    break;
                }
                operator = buf[i]; // ustawienie operatora
            }
            else //jeśli znak jest niepoprawny tj. nie jest cyfrą ani operatorem
            {
                error = true;
                break;
            }
            
        }
        if(!error)
        {
            if(buf[bajty-1] == '+' || buf[bajty-1] == '-')
            {
                error = true;
            }
        }
        // wykonanie ostatniego działania
        if(operator == '+') // wykonanie działania
        {
            if(wynik <= 0 || (INT16_MAX - wynik >= liczba)) // sprawdzanie czy nie przekroczymy zakresu
            {
                wynik += liczba;
                liczba = 0;
            }
            else
            {
                error = true;
            }
        }
        else if(operator == '-') // wykonanie działania
        {
            if(wynik >= 0 || (wynik - INT16_MIN >= liczba)) // sprawdzanie czy nie przekroczymy zakresu
            {
                wynik -= liczba;
                liczba = 0;
            }
            else
            {
                error = true;
            }
        }
        else
        {
            error = true;
        }
        
        if(error) // jeśli wystąpił błąd
        {
            if(memcpy(buf, "ERROR", 5) == NULL) // kopiowanie błędu do bufora
            {
                perror("Nie udało się skopiować danych");
                exit(1);
            }
            if((sendto(gniazdko, buf, 5, 0, (struct sockaddr *) &client_addr, client_addr_len)) == -1) // wysłanie błędu
            {
                perror("Nie udało się wysłać danych");
                exit(1);
            }
        }
        else // jeśli nie wystąpił błąd
        {
            char wynikliczba[20]; // bufor na wynik
            if(sprintf(wynikliczba , "%d" , wynik) < 0) // konwersja wyniku na string
            {
                perror("Nie udało się skopiować danych");
                exit(1);
            }
            if(memcpy(buf, wynikliczba, sizeof(wynikliczba)) == NULL) // kopiowanie wyniku do bufora
            {
                perror("Nie udało się skopiować danych");
                exit(1);
            }
            if((sendto(gniazdko, buf, strlen(wynikliczba), 0, (struct sockaddr *) &client_addr, client_addr_len)) == -1) // wysłanie wyniku
            {
                perror("Nie udało się wysłać danych");
                exit(1);
            }
        }
    }
}