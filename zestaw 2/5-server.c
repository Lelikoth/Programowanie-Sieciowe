//Napisz prosty serwer zwracający wizytówkę. Powinien tworzyć gniazdko TCP nasłuchujące na porcie o numerze podanym 
//jako argv[1] (użyj socket, bind i listen), następnie w pętli czekać na przychodzące połączenia (accept), 
//wysyłać ciąg bajtów Hello, world!\r\n jako swoją wizytówkę, zamykać odebrane połączenie i wracać na początek pętli. 
//Pętla ma działać w nieskończoność, aby przerwać działanie programu trzeba będzie użyć Ctrl-C

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

int gniazdko = -1;

struct sockaddr_in* adres;

void zamknij_gniazdko(){
    if(gniazdko != -1){
        close(gniazdko);
    }

    if(adres != NULL){
        free(adres);
    }
}


int main(int argc, char** argv){
    if(argc != 2){
        printf("Niepoprawna liczba argumentow!\n");
        return 1;
    }

    int port = atoi(argv[1]);

    if(port < 1025 || port > 65535){
        printf("Niepoprawny numer portu!\n");
        return 1;
    }

    adres = malloc(sizeof(struct sockaddr_in));

    if(adres == NULL){
        printf("Nie udalo sie zaalokowac pamieci!\n");
        return 1;
    }

    adres->sin_family = AF_INET;
    adres->sin_port = htons(port);
    adres->sin_addr.s_addr = htonl(INADDR_ANY);

    gniazdko = socket(AF_INET, SOCK_STREAM, 0);

    if(gniazdko == -1){
        printf("Nie udalo sie utworzyc gniazda!\n");
        return 1;
    }

    if(bind(gniazdko, (struct sockaddr*)adres, sizeof(struct sockaddr_in)) == -1){
        printf("Nie udalo sie przypisac adresu do gniazda!\n");
        return 1;
    }

    if(listen(gniazdko, 10) == -1){
        printf("Nie udalo sie ustawic gniazda w tryb nasluchiwania!\n");
        return 1;
    }

    while(1){
        int gniazdko_klienta = accept(gniazdko, NULL, NULL);

        if(gniazdko_klienta == -1){
            printf("Nie udalo sie utworzyc gniazda dla klienta!\n");
            return 1;
        }

        if(write(gniazdko_klienta, "Hello, world!\r\n", 15) == -1){
            printf("Nie udalo sie wyslac wiadomosci do klienta!\n");
            return 1;
        }

        if(close(gniazdko_klienta) == -1){
            printf("Nie udalo sie zamknac gniazda dla klienta!\n");
            return 1;
        }
    }

    return 0;

}
