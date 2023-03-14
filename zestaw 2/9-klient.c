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

int gniazdko = -1;

bool drukowalne (const void* buf, size_t len) {
    const char* b = buf;
    for (size_t i = 0; i < len; i++) {
        if (b[i] < 32 || b[i] > 126) {
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

    const char* adres_ip = argv[1];
    int port = atoi(argv[2]);

    atexit(zamknij_gniazdko);

    struct sockaddr_in* adres = malloc(sizeof(struct sockaddr_in));
    adres->sin_family = AF_INET;
    adres->sin_port = htons(port);
    adres->sin_addr.s_addr = inet_addr(adres_ip);

    gniazdko = socket(AF_INET, SOCK_DGRAM, 0);
    if(gniazdko == -1){
        printf("Nie udalo sie utworzyc gniazda!\n");
        exit(1);
    }

    if(sendto(gniazdko, "", 0, 0, (struct sockaddr*)adres, sizeof(struct sockaddr_in)) == -1){
        printf("Nie udalo sie wyslac wiadomosci!\n");
        exit(1);
    }

    char buf[1024];
    socklen_t dlugosc_adresu = sizeof(adres);

    if(recvfrom(gniazdko, buf, 1024, 0, (struct sockaddr*)adres, &dlugosc_adresu) == -1){
        printf("Nie udalo sie odebrac wiadomosci!\n");
        exit(1);
    }

    if(drukowalne(&buf, 1024)){
        printf("%s\n", buf);
    }else{
        printf("Wiadomosc nie jest drukowalna!\n");
        exit(1);
    }

    if(close(gniazdko) == -1){
        printf("Nie udalo sie zamknac gniazda!\n");
        exit(1);
    }
}
