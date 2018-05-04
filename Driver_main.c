/* DRIVER using SUSI for the MOVEBOX */
/* BALYO */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)

#define PORT	 	75
#define MAX_CLIENTS 	100

#define BUF_SIZE	1024

#include <ctype.h>
#include <termios.h>
#include <sys/fcntl.h>
#include <inttypes.h>
#include <signal.h>

#include "SUSIDriver/Susi4.h"
#include "DriverLibrary.h"
#include "AssistantLibrary.h"



int main(int argc, char *argv[]){
    /** Variables Driver **/
    int i;
    int connected = 0;
    int choice;
    unsigned char grandeTrame[NumberOfBits];
    unsigned char frame4test[NumberOfBits];
    unsigned int paquet = 0;
    for(i = 0; i < NumberOfBits ;i++){ // Initialization trame
        grandeTrame[i] = 0;
    }
    /** Beginning **/
    connected = initializationDriver(&grandeTrame[0]);
    if(connected){
        miseAJourEnteteTrame(&grandeTrame[0]);
        printf("Local frame:\n");
        printTrameBinary(&grandeTrame[0]); //Affichage de la trame pour connaitre son structure
        do{
            choice = menu();
            writeGPIO(&grandeTrame[0], &paquet, choice,&buffer[0]);
            readGPIO(&grandeTrame[0]);
            printTrameHex(&grandeTrame[0]);
            printf("Do you want to leave? [1|0]");
            scanf("%d", &choice);

        }while(choice != 1);
        leaveSUSI();
    }
    return 0;
}


