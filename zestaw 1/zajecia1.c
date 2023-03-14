#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


void drukuj(int tab[], int liczba_elementow)
{
    for (int i = 0; i < liczba_elementow; i++)
    {
        if (tab[i] > 10 && tab[i] < 100)
        printf("%d ", tab[i]);
    }
    printf("\n");
}

void drukuj_alt(int * tablica, int liczba_elementow)
{
    int *ptr = tablica;

    while( ptr < tablica + liczba_elementow )
    {
        if (*ptr > 10 && *ptr < 100)
        printf("%d ", *ptr);
        ptr++;
    }
    printf("\n");
}

bool printable_buf(const void * buf, int len)
{
    const char *ptr = buf;

    for (int i = 0; i < len; i++)
    {
        if (*ptr < 32 || *ptr > 126)
            return false;
        ptr++;
    }
    return true;
}

bool printable_str(const char * buf)
{
    const char *ptr = buf;

    while (*ptr != '\0')
    {
        if (*ptr < 32 || *ptr > 126)
            return false;
        ptr++;
    }
    return true;
}


int main(int argc, char *argv[])
{
    // Zadanie 1
    int tab[50];
    int i = 0;

    while (scanf("%d", &tab[i]) != EOF) {
        if (i == 49)
        {
            printf("Maksymalna pojemnosc osiagnieta!");
            break;
        }
        if(tab[i] == 0)
            break;
        i++;
    }

    drukuj(tab, i+1);

    // Zadanie 2

    drukuj_alt(tab, i+1);

    // Zadanie 3

    char buf[3] = {'1', 'a', '?'};
    int len = 3;
    for (int i = 0; i < len; i++)
        printf("%d ", buf[i]);

    printf("\n%s\n", printable_buf(buf, len) ? "true" : "false");

    // Zadanie 4

    printf("%s\n", printable_str(buf) ? "true" : "false");

    // Zadanie 5
    // funkcja read() zwraca liczbe odczytanych bajtow, jeżeli jest mniejsza od 0, to znaczy, że wystąpił błąd
    // jeżeli funkcja zwróci 0, to znaczy, że nie ma więcej danych do odczytu

    // Zadanie 6
    
    int fin = open(argv[1], O_RDONLY);
    if (fin == -1)
    {
        printf("Blad otwarcia pliku: %s\n", strerror(errno));
        return 1;
    }

    int fout = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fout == -1)
    {
        printf("Blad otwarcia pliku: %s\n", strerror(errno));
        return 1;
    }

    char buf2[1024];
    char *ptr;
    int n;

    while ((n = read(fin, buf2, 1024)) > 0)
    {
        ptr = buf2;
        while (n > 0)
        {
            int n2 = write(fout, ptr, n);
            if (n2 == -1)
            {
                printf("Blad zapisu do pliku: %s\n", strerror(errno));
                return 1;
            }
            n -= n2;
            ptr += n2;
        }
    }
    if(n == -1)
    {
            printf("Blad odczytu z pliku: %s\n", strerror(errno));
            return 1;
    }
    if (close(fin) == -1)
        printf("Blad zamkniecia pliku: %s\n", strerror(errno));

    if (close(fout) == -1)
        printf("Blad zamkniecia pliku: %s\n", strerror(errno));
    
    // Zadanie 7: Modyfikacja powyższego zadania. Zakładamy, że kopiowany plik jest plikiem tekstowym. Linie są zakończone bajtami o wartości 10 (znaki LF, w języku C zapisywane jako '\n'). Podczas kopiowania należy pomijać parzyste linie (tzn. w pliku wynikowym mają się znaleźć pierwsza, trzecia, piąta linia, a druga, czwarta, szósta nie).


    fin = open(argv[3], O_RDONLY);
    if (fin == -1)
    {
        printf("Blad otwarcia pliku: %s\n", strerror(errno));
        return 1;
    }

    fout = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fout == -1)
    {
        printf("Blad otwarcia pliku: %s\n", strerror(errno));
        return 1;
    }

    char buf3[1024];
    int line = 1;
    int n2;

    while ((n = read(fin, buf3, sizeof(buf3))) > 0)
    {
        ptr = buf3;
        while (n > 0)
        {
            while(*ptr != '\n')
            {
                if (line % 2 == 1)
                {
                    n2 = write(fout, ptr, 1);
                    if (n2 == -1)
                    {
                        printf("Blad zapisu do pliku: %s\n", strerror(errno));
                        return 1;
                    }
                    n -= n2;
                    ptr += n2;
                }
            }
            line++;
            ptr++;
            n--;
               
        }
        if(n == -1)
        {
            printf("Blad odczytu z pliku: %s\n", strerror(errno));
            return 1;
        }
    }
}