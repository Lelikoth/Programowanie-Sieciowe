//Napisz prostego klienta, który łączy się (użyj socket i connect) z usługą wskazaną 
//argumentami podanymi w linii komend (adres IPv4 w argv[1], numer portu TCP w argv[2]), 
//drukuje na ekranie wizytówkę zwróconą przez serwer i kończy pracę. Pamiętaj o zasadzie 
//ograniczonego zaufania i przed przesłaniem odebranego bajtu na stdout weryfikuj, 
//czy jest to znak drukowalny lub znak kontrolny używany do zakończenia linii bądź wstawienia odstępu ('\n', '\r' oraz '\t').

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>

int gniazdko = -1;

struct sockaddr_in* adres = NULL;

void zamknij_gniazdko(){
    if(gniazdko != -1){
        close(gniazdko);
    }

    if(adres != NULL){
        free(adres);
    }
}

bool drukowalny (const void* buf, int len) {
    
    const char* _buf = buf;

    for (int i = 0; i < len; i++) {
        if (_buf[i] < 32 || _buf[i] > 126) {
            return false;
        }
    }

    return true;
}

int main(int argc, char const *argv[]) {

    const char* adres_ip = argv[1];
    int port = atoi(argv[2]);

    adres = malloc(sizeof(struct sockaddr_in));
    if (adres == NULL) {
        printf("Nie udalo sie zaalokowac pamieci!\n");
        return 1;
    }

    adres->sin_family = AF_INET;
    adres->sin_port = htons(port);
    adres->sin_addr.s_addr = inet_addr(adres_ip);

    gniazdko = socket(AF_INET, SOCK_STREAM, 0);
    if (gniazdko == -1) {
        printf("Nie udalo sie utworzyc gniazda!\n");
        return 1;
    }

    if (connect(gniazdko, (struct sockaddr*)adres, sizeof(struct sockaddr_in)) == -1) {
        printf("Nie udalo sie nawiazac polaczenia!\n");
        return 1;
    }

    char buf[15];

    if (read(gniazdko, buf, 15) == -1) {
        printf("Nie udalo sie odczytac danych!\n");
        return 1;
    }

    if (drukowalny(buf, 13)) {
        printf("%s\n", buf);
    } else {
        printf("Niedrukowalne znaki w buforze!\n");
        return 1;
    }

    if(close(gniazdko) == -1){
        printf("Nie udalo sie zamknac gniazda!\n");
        return 1;
    }

    return 0;
}