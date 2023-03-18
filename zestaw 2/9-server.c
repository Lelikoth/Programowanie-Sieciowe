//Spróbuj napisać parę klient-serwer komunikującą się przy pomocy protokołu UDP. 
//Pamiętaj o tym, że UDP nie nawiązuje połączeń, więc to klient będzie musiał jako pierwszy wysłać jakiś datagram, 
//a serwer na niego odpowie. Sprawdź, czy możliwe jest wysyłanie pustych datagramów (tzn. o długości zero bajtów) — jeśli tak, 
//to może niech klient właśnie taki wysyła?

//Serwer:

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

int gniazdko = -1;

void zamknij_gniazdko(){
    if(gniazdko != -1){
        close(gniazdko);
    }
}

int main(int argc, char const *argv[]){

    int port = atoi(argv[1]);

    atexit(zamknij_gniazdko);

    struct sockaddr_in adres = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
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

    unsigned char buf[1024];

    while(1){
        struct sockaddr_in klient;
        socklen_t dlugosc_adresu = sizeof(klient);

        if(recvfrom(gniazdko, buf, 1024, 0, (struct sockaddr*)&klient, &dlugosc_adresu) == -1){
            printf("Nie udalo sie odebrac wiadomosci!\n");
            exit(1);
        }

        if(sendto(gniazdko, "Hello World\n", 12, 0, (struct sockaddr*)&klient, dlugosc_adresu) == -1){
            printf("Nie udalo sie wyslac wiadomosci!\n");
            exit(1);
        }
    }
}
