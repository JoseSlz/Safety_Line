#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <signal.h>

#include "SUSIDriver/Susi4.h"
#include "AssistantLibrary.h"
#include "DriverLibrary.h"

void printTrameBinary(unsigned char *grandeTrame){
    int i;
    printf("\nName\t0x\n");
    for(i = 0; i < NumberOfBits;i ++ ){

        if(i == 0) printf("\nStart\t");
        if(i == type) printf("\nType\t");
        if(i == size) printf("\nSize\t");
        if(i == CRC ) printf("\nCRC\t");
        if(i == PAQUET) printf("\nPaquet\t");
        if(i == MODE) printf("\nMode\t");
        if(i == version) printf("\nVersion\t");
        if(i == SortiesN) printf("\nSorties\t");
        if(i == EntreesN) printf("\nEntrees\t");
        printf("%d", grandeTrame[i]);
    }
    printf("\n");
}

void printTrameHex(unsigned char *grandeTrame){
    printf("Start\tType\tSize\tCRC\t\tPaquet\t(Manual/Auto)\tVersion\tSorties\tEntrees\t\n");
    printf("0x");
    printf("%c%c\t", binary2hex(&grandeTrame[start]), binary2hex(&grandeTrame[start+4]));
    printf("%c%c\t", binary2hex(&grandeTrame[type]),binary2hex(&grandeTrame[type+4]));
    printf("%c%c%c%c\t", binary2hex(&grandeTrame[size]), binary2hex(&grandeTrame[size+4]), binary2hex(&grandeTrame[size+8]), binary2hex(&grandeTrame[size+12]));
    printf("%c%c%c%c%c%c%c%c\t", binary2hex(&grandeTrame[CRC]),binary2hex(&grandeTrame[CRC+4]),binary2hex(&grandeTrame[CRC+8]),binary2hex(&grandeTrame[CRC+12]),binary2hex(&grandeTrame[CRC+16]),binary2hex(&grandeTrame[CRC+20]),binary2hex(&grandeTrame[CRC+24]),binary2hex(&grandeTrame[CRC+28]));
    printf("%c%c%c%c%c%c%c%c\t", binary2hex(&grandeTrame[PAQUET]),binary2hex(&grandeTrame[PAQUET+4]),binary2hex(&grandeTrame[PAQUET+8]),binary2hex(&grandeTrame[PAQUET+12]),binary2hex(&grandeTrame[PAQUET+16]),binary2hex(&grandeTrame[PAQUET+20]),binary2hex(&grandeTrame[PAQUET+24]),binary2hex(&grandeTrame[PAQUET+28]));
    printf("%c%c\t", binary2hex(&grandeTrame[MODE]), binary2hex(&grandeTrame[MODE+4]));
    printf("%c%c%c%c\t", binary2hex(&grandeTrame[version]), binary2hex(&grandeTrame[version+4]), binary2hex(&grandeTrame[version+8]), binary2hex(&grandeTrame[version+12]));
    printf("%c%c%c%c\t", binary2hex(&grandeTrame[SortiesN]), binary2hex(&grandeTrame[SortiesN+4]), binary2hex(&grandeTrame[SortiesN+8]), binary2hex(&grandeTrame[SortiesN+12]));
    printf("%c%c%c%c%c%c\t",binary2hex(&grandeTrame[EntreesN]), binary2hex(&grandeTrame[EntreesN +4]), binary2hex(&grandeTrame[EntreesN +8]), binary2hex(&grandeTrame[EntreesN +12]), binary2hex(&grandeTrame[EntreesN +16]), binary2hex(&grandeTrame[EntreesN +20]));
    printf("\n");
}


void clearTrame(unsigned char *Trame){
    for(int i=0; i < NumberOfBits; i++ ){
        Trame[i] = 0;
    }
}

void dec2bin(unsigned char *grandeTrame, int adresseSuivante, int nombre){
    int i = 0;
    do{
       grandeTrame[adresseSuivante - 1 + i] = 0 + nombre%2;
       nombre = nombre/2;
       i--;
    }while (nombre != 0);


}

char binary2hex(unsigned char *valeur){
    char result;
    if(valeur[0] == 0 && valeur[1] == 0 && valeur [2] == 0 && valeur[3] == 1 ){
        result = '1';
    }else{
        if(valeur[0] == 0 && valeur[1] == 0 && valeur [2] == 1 && valeur[3] == 0){
            result = '2';
        }else{
            if(valeur[0] == 0 && valeur[1] == 0 && valeur [2] == 1 && valeur[3] == 1 ){
                result = '3';
            }else{
                if(valeur[0] == 0 && valeur[1] == 1 && valeur [2] == 0 && valeur[3] == 0 ){
                        result = '4';

                }else{
                    if(valeur[0] == 0 && valeur[1] == 1 && valeur [2] == 0 && valeur[3] == 1){
                            result = '5';

                    }else{
                        if(valeur[0] == 0 && valeur[1] == 1 && valeur [2] == 1 && valeur[3] == 0){
                                result = '6';

                        }else{
                            if(valeur[0] == 0 && valeur[1] == 1 && valeur [2] == 1 && valeur[3] == 1 ){
                                    result = '7';

                            }else{
                                if(valeur[0] == 1 && valeur[1] == 0 && valeur [2] == 0 && valeur[3] == 0 ){
                                        result = '8';

                                }else{
                                    if(valeur[0] == 1 && valeur[1] == 0 && valeur [2] == 0 && valeur[3] == 1 ){
                                            result = '9';

                                    }else{
                                        if(valeur[0] == 1 && valeur[1] == 0 && valeur [2] == 1 && valeur[3] == 0 ){
                                                result = 'A';

                                        }else{
                                            if(valeur[0] == 1 && valeur[1] == 0 && valeur [2] == 1 && valeur[3] == 1 ){
                                                    result = 'B';

                                            }else{
                                                if(valeur[0] == 1 && valeur[1] == 1 && valeur [2] == 0 && valeur[3] == 0 ){
                                                        result = 'C';

                                                }else{
                                                    if(valeur[0] == 1 && valeur[1] == 1 && valeur [2] == 0 && valeur[3] == 1 ){
                                                            result = 'D';

                                                    }else{
                                                        if(valeur[0] == 1 && valeur[1] == 1 && valeur [2] == 1 && valeur[3] == 0 ){
                                                                result = 'E';

                                                        }else{
                                                            if(valeur[0] == 1 && valeur[1] == 1 && valeur [2] == 1 && valeur[3] == 1 ){
                                                                    result = 'F';

                                                            }else{
                                                                if(valeur[0] == 0 && valeur[1] == 0 && valeur [2] == 0 && valeur[3] == 0 ){
                                                                    result = '0';
                                                                }

                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}
