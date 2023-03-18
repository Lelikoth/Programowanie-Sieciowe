//Napisz serwer UDP/IPv4 nasłuchujący na porcie nr 2020 i implementujący powyższy protokół.
//Serwer musi weryfikować odebrane dane i zwracać komunikat o błędzie jeśli są one nieprawidłowe w sensie zgodności ze specyfikacją protokołu.
//W kodzie używaj zmiennych roboczych któregoś ze standardowych typów całkowitoliczbowych (int, long int, int32_t, itd.).
//Co za tym idzie, odebrany ciąg cyfr będzie mógł reprezentować liczbę zbyt dużą, aby dało się ją zapisać w zmiennej wybranego typu. 
//Może też się zdarzyć, że podczas dodawania bądź odejmowania wystąpi przepełnienie (ang. integer overflow / wraparound). 
//Serwer ma obowiązek wykrywać takie sytuacje i zwracać błąd. Uwadze Państwa polecam pliki nagłówkowe limits.h oraz stdint.h, 
//w których znaleźć można m.in. parę stałych INT_MIN i INT_MAX oraz parę INT32_MIN i INT32_MAX.


#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

#define MAX_DATA 65527

int gniazdko = -1;

void zamknij_gniazdko(){
    if(gniazdko != -1){
        close(gniazdko);
    }
}

int main(){

    //zamykanie gniazda przy wyjsciu z programu lub bledzie
    atexit(zamknij_gniazdko);

    struct sockaddr_in adres = {
        .sin_family = AF_INET,
        .sin_port = htons(2020),
        .sin_addr.s_addr = htonl(INADDR_ANY)
    };

    gniazdko = socket(AF_INET, SOCK_DGRAM, 0);
    if(gniazdko == -1){
        printf("Nie udalo sie utworzyc gniazda!\n");
        exit(1);
    }

    if(bind(gniazdko, (struct sockaddr*)&adres, sizeof(struct sockaddr_in)) == -1){
        printf("Nie udalo sie przypisac adresu do gniazda!\n");
        exit(1);
    }
    
    while(1){

        struct sockaddr_in klient;
        socklen_t klient_rozmiar = sizeof(klient);

        //bufor na dane
        char buf[MAX_DATA+1];
        buf[MAX_DATA] = '\n';
        unsigned long int suma = 0;
        char odczytana_liczba[MAX_DATA];
        bool netcat = false;
        bool blad = false;

        //ramka
        if(recvfrom(gniazdko, buf, sizeof(buf), 0, (struct sockaddr*)&klient, &klient_rozmiar) == -1){
            printf("Nie udalo sie odebrac danych!\n");
            exit(1);
        }

        //odebranie danych
        for(int i = 0, j = 0; i < MAX_DATA; i++){

            //znak konca liczby spacja, \n, \r\n
            if(buf[i] == ' ' || buf[i] == '\n' || buf[i] == '\r'){
                odczytana_liczba[j] = '\0';

                unsigned long int liczba = strtoul(odczytana_liczba, NULL, 10);
                //sprwadzenie czy liczba została poprawnie przekonwertowana
                unsigned long int test = liczba;
                for(int k = j - 1; k >= 0; k--){
                    if(odczytana_liczba[k] != (test % 10) + '0'){
                        blad = true;
                        break;
                    }
                    test /= 10;
                }

                //sprawdzenie czy liczba mieści się w typie int
                if(suma > suma + liczba || blad == true){
                    blad = true;
                    break;
                }

                suma += liczba;
                j = 0;

                //jesli zakonczenie lini \n to netcat
                if(buf[i] == '\n'){
                    netcat = true;
                    break;
                }
            }
            else if(buf[i] == '\n' || (buf[i] == '\r' && buf[i+1] == '\n')){
                netcat = true;
                break;
            }
            else if (buf[i] >= '0' && buf[i] <= '9'){
                odczytana_liczba[j] = buf[i];
                j++;
                //sprawdzenie czy liczba nie jest za dluga
                if(j > 10){
                    blad = true;
                    break;
                }
            }
            else{
                blad = true;
                break;
            }
        }

        if(!blad){
            int dlugosc_odpowiedz = 0;
            if(netcat){
                dlugosc_odpowiedz = sprintf(odczytana_liczba, "%ld\n", suma);
            }
            else{
                dlugosc_odpowiedz = sprintf(odczytana_liczba, "%ld", suma);
            }

            if(sendto(gniazdko, odczytana_liczba, dlugosc_odpowiedz, 0, (struct sockaddr*)&klient, klient_rozmiar) == -1){
                printf("Nie udalo sie wyslac danych!\n");
                exit(1);
            }
        }
        else{
            if(sendto(gniazdko, "ERROR\n", 6, 0, (struct sockaddr*)&klient, klient_rozmiar) == -1){
                printf("Nie udalo sie wyslac danych!\n");
                exit(1);
            }
        }
    }
}