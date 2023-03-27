//Spróbuj napisać parę klient-serwer komunikującą się przy pomocy protokołu UDP. 
//Pamiętaj o tym, że UDP nie nawiązuje połączeń, więc to klient będzie musiał jako pierwszy wysłać jakiś datagram, 
//a serwer na niego odpowie. Sprawdź, czy możliwe jest wysyłanie pustych datagramów (tzn. o długości zero bajtów) — jeśli tak, 
//to może niech klient właśnie taki wysyła?

//Klient:

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <string.h>

int gniazdko = -1;

bool drukowalne (const void* buf, int len) {
    
    const char* _buf = buf;

    for (int i = 0; i < len; i++) {
        if(_buf[i] == '\n' || _buf[i] == '\t' || _buf[i] == '\r')
        {
            continue;
        }
        else if (_buf[i] < 32 || _buf[i] > 126) {
            return false;
        }
    }

    return true;
}

void zamknij_gniazdko(){
    if(gniazdko != -1){
        close(gniazdko);
    }
}


int main(int argc, char const *argv[]){

    const char* adres_ip = "127.0.0.1";
    int port = 2020;

    atexit(zamknij_gniazdko);

    struct sockaddr_in adres = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = inet_addr(adres_ip)
    };

    gniazdko = socket(AF_INET, SOCK_DGRAM, 0);
    if(gniazdko == -1){
        printf("Nie udalo sie utworzyc gniazda!\n");
        exit(1);
    }

    if(sendto(gniazdko, "123-23+1+\n", 10, 0, (struct sockaddr*)&adres, sizeof(struct sockaddr_in)) == -1){
        printf("Nie udalo sie wyslac wiadomosci!\n");
        exit(1);
    }

    char buf[40];
    strcpy(buf, "");
    socklen_t dlugosc_adresu = sizeof(adres);

    if(recvfrom(gniazdko, buf, 40, 0, (struct sockaddr*)&adres, &dlugosc_adresu) == -1){
        printf("Nie udalo sie odebrac wiadomosci!\n");
        exit(1);
    }

    for (int i = 0; i < 40; i++) {
        printf("%c ", buf[i]);
    }
}
