#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <signal.h>
#include <zlib.h>

#include "SUSIDriver/Susi4.h"

#include "DriverLibrary.h"

#include "AssistantLibrary.h"

// Function iic_probe taken from TestMovebox
int iic_probe(uint8_t iDevice, int addrType){ // Looking for Implementation Components on the bus
	SusiId_t id = devids[iDevice];
	SusiStatus_t status;
	uint32_t shiftaddr;
	uint16_t i;
    int PCA_Present = 0x00; // Principal Components Analysis
		for (i = 0x03; i < 0x78; i++)
		{
			shiftaddr = SUSI_I2C_ENC_7BIT_ADDR(i);
			status = SusiI2CProbeDevice(id, shiftaddr);
			if (status == SUSI_STATUS_SUCCESS)
			{
                if (shiftaddr == 0x40) PCA_Present = PCA_Present | 0x01;
                if (shiftaddr == 0x42) PCA_Present = PCA_Present | 0x02;
			}
		}

    if (PCA_Present == 0 )  PCA_Present = -1;

	return PCA_Present;    // PCA_Present = -1 si les IC ne sont pas sur le BUS I2C
	                       // ...1 si le IC d'entreé est present (mais pas le IC de sortie)
	                       // ...2 si le IC de sortie est present (mais pas le IC d'entrée)
	                       // ...3 si les 2 IC sont presents.
}


int initializationDriver(unsigned char *grandeTrame){
    int choice;
    int connected = 0; // Flag to know if the Driver has been well initialized
    SusiStatus_t status;
    int succesGPIO = 1;
    system("clear");
    printf("*****************************************************************************************************\n");
    printf("                                            DRIVER MOVEBOX                                           \n");
    printf("*****************************************************************************************************\n\n");
    do{
        printf("Initialize the driver? [Y(1)/n(0)]: ");
        scanf("%d", &choice);
        if (choice == 1){
            // Values' actualization on frame
            grandeTrame[0] = 1;
            grandeTrame[7] = 1;
            printf("Starting SUSI... \n");
            status = SusiLibInitialize();
            if (status == SUSI_STATUS_ERROR) // Test for check the driver initialization
            {
                printf("You must execute the next command: (# sudo ./DriverProto2)!\n");
                printf("Aborting!!.\n");
            }
            if (status != SUSI_STATUS_SUCCESS && status != SUSI_STATUS_INITIALIZED )
            {
                printf("SusiLibInitialize() failed. (0x%08X)\n", status);
                printf("Exit the program...\n");
            }
            else
            {
                if (status == SUSI_STATUS_SUCCESS || status == SUSI_STATUS_INITIALIZED){
                    printf("SUSI: Initialization succeed\n\n");
                    connected = 1;
                }

            }
        }else{
            if(choice == 0){
                printf("\n\nLeaving program...\n...\n...\n");
                printf("See you soon\n");
            }else{
                printf("\nWrong command, please retry\n");
            }

        }
    }while(choice != 0 && choice != 1);
    succesGPIO = confGPIO();
    if(!succesGPIO) connected = 0;
    return connected;
}

int selectionMode(unsigned char *grandeTrame){
    int choice;
    int mode = 0;
    do{
        printf("Automatic/Manual mode[1|0]: ");
        scanf("%d", &choice);
        getchar();
        if(choice == 1){
            grandeTrame[version - 1] = 1;
            mode = 1;
        }else{
            if(choice == 0){
                printf("Manual mode activated\n");
                grandeTrame[version - 1] = 0;
            } else {
                printf("Wrong command, please retry\n\n");
            }
        }
    }while(choice != 0 && choice != 1);
    return mode;
}

int confGPIO(){
    int i;
    SusiStatus_t status;
    int PCA;
    int counter = 0;
    int succesGPI = 1;
    uint8_t configurationMask;
    struct I2CConf config;
    SusiId_t id = 0;
    printf("*****************************************************************************************************\n");
    printf("                                     INPUTS/OUTPUTS CONFIGURATION        \n");
    printf("*****************************************************************************************************\n");
    status = SusiGPIOSetLevel(BalyoGPIO_Reset, GPIO_OutputMask, LOW);
    if (status == SUSI_STATUS_SUCCESS) counter++;
    status = SusiGPIOSetLevel(BalyoGPIO_Reset, GPIO_OutputMask, HIGH);
    if (status == SUSI_STATUS_SUCCESS) counter++;
    status = SusiGPIOSetLevel(BalyoGPIO_OutputEnable, GPIO_OutputMask, LOW);
    if (status == SUSI_STATUS_SUCCESS) counter++;
    if(counter == 3){
        printf("[SUSI-] I2C device enabled\n");
        printf("[SUSI-] Looking for I2C device...\n");
        PCA = iic_probe(0, 0);
        printf("PCA: %d, Then:\n", PCA);
        switch(PCA){
            case -1: printf("The IC are on the BUS I2C.\n");            // Implementation Component = IC
                     break;
            case 1:  printf("Only the input IC is present.\n");
                     break;
            case 2:  printf("Only the output IC is present.\n");
                     break;
            case 3:  printf("INPUT AND OUTPUT IC presents.\n");
                     break;
            default: printf("An error has occurred.\n");
                     break;
        }
        if(PCA == 3){
            printf("[SUSI-] I2C device found\n");
            printf("[SUSI-] Configuring inputs...\n");
            /*** Configuring GPI ***/
            configurationMask = 0xFF; // Input address
            config.addr = GPI_Addr;
            config.wdata = &configurationMask;
            config.wlen = 1;
            for(i = 0; i < NumberOfBanks && status == SUSI_STATUS_SUCCESS; i++){
                printf("[SUSI-] BANK %d...\n", i);
                status = SusiI2CWriteTransfer(id, config.addr, I2C_Bank_Configuration[i], config.wdata, config.wlen);
            }
            if(status == SUSI_STATUS_SUCCESS){
                printf("GPI OK (PCA9505/06) \n\n");
            } else {
                printf("GPI was not configured... please retry \n\n");
                succesGPI *= 0;
            }
            /*** Configuring GPO ***/
            config.addr = GPO_Addr;
            config.cmd = GPIO_Conf_1;
            configurationMask = 0x00; // Output address
            config.wdata = &configurationMask;
            config.wlen = 1;
            for(i = 0; i < NumberOfBanks && status == SUSI_STATUS_SUCCESS; i++){
                printf("[SUSI-] BANK %d...\n", i);
                status = SusiI2CWriteTransfer(id, config.addr, I2C_Bank_Configuration[i], config.wdata, config.wlen);
            }
            if(status == SUSI_STATUS_SUCCESS){
                printf("GPO OK (PCA9505/06) \n\n");
            } else {
                printf("GPO was not configured... please retry \n\n");
                succesGPI *= 0;
            }
        }
    }
    if(succesGPI) printf("[SUSI-] I2C inputs and outputs configured\n\n");
    printf("*****************************************************************************************************\n");
    return succesGPI;
}

int writeGPIO(unsigned char *grandeTrame, unsigned int *paquet, int choice, unsigned char *trameRecu){
    int succeswriting = 1;
    int i, j;
    int ValidValues[4] = {3,1,0,-1}; //{HiZ(3rdState),High, Low, Error}
    SusiStatus_t status;
    struct I2CConf config;
    int counter = 0;
    uint8_t writedatabuffer[0x100]; //Input buffer
    SusiId_t id = 0;
    status = SusiGPIOSetLevel(BalyoGPIO_OutputEnable, GPIO_OutputMask, LOW);
    if( status == SUSI_STATUS_SUCCESS ) counter++;
    /************ Trying to write something ***************/
    printf("\n***********SENDING DATA**********\n\n");
    /** Write BANK 0 **/
    config.addr = GPO_Addr; //GPIO Output Address -voir BalyoLowLevel.h-
    config.wlen = 1; //1 octet a ecrire
    config.cmd = GPO_0;
    writedatabuffer[0] = buffer2write(choice, 0, &trameRecu[0]); // 0x55 //0x00 // 0xFF
    status = SusiI2CWriteTransfer(id, config.addr, config.cmd, writedatabuffer, config.wlen);
    if( status == SUSI_STATUS_SUCCESS ) counter++;
    if(counter == 2){
        printf("SUCCES while writing data on BANK 0\n");
        printf("Data submitted: ");
        printf("0x%02X\n", writedatabuffer[0]);
        j = 0;
        /** Actualization of OUTPUTS on frame **/
        grandeTrame[SortiesN + (j++)] = ValidValues[(writedatabuffer[0]>>0) & 0x03];
        grandeTrame[SortiesN + (j++)] = ValidValues[(writedatabuffer[0]>>2) & 0x03];
        grandeTrame[SortiesN + (j++)] = ValidValues[(writedatabuffer[0]>>4) & 0x03];
        grandeTrame[SortiesN + (j++)] = ValidValues[(writedatabuffer[0]>>6) & 0x03];
    }
    /*** Write BANK 1**/
    writedatabuffer[0] = 0x00;
    writedatabuffer[0] = buffer2write(choice, 1, &trameRecu[0]);
    config.cmd = GPO_1;
    status = SusiI2CWriteTransfer(id, config.addr, config.cmd, writedatabuffer, config.wlen);
    if(status == SUSI_STATUS_SUCCESS){
        printf("SUCCES while writing data on BANK 1\n");
        printf("Data submitted: ");
        printf("0x%02X\n", writedatabuffer[0]);
        j = 0;
        grandeTrame[SortiesN + 4 + (j++)] = ValidValues[(writedatabuffer[0]>>0) & 0x03];
        grandeTrame[SortiesN + 4 + (j++)] = ValidValues[(writedatabuffer[0]>>2) & 0x03];
        grandeTrame[SortiesN + 4 + (j++)] = ValidValues[(writedatabuffer[0]>>4) & 0x03];
        grandeTrame[SortiesN + 4 + (j++)] = ValidValues[(writedatabuffer[0]>>6) & 0x03];
    }
    if(status == SUSI_STATUS_WRITE_ERROR){
        printf("An error has occurred while writing data...\n");
        succeswriting = 0;
    }
    /*** Write BANK 2***/
    config.cmd = GPO_2;
    writedatabuffer[0] = 0x00;
    writedatabuffer[0]= buffer2write(choice, 2, &trameRecu[0]);
    status = SusiI2CWriteTransfer(id, config.addr, config.cmd , writedatabuffer, config.wlen);
    if(status == SUSI_STATUS_SUCCESS){
        printf("SUCCES while writing data on BANK 2\n");
        printf("Data submitted: ");
        printf("0x%02X\n", writedatabuffer[0]);
        j = 0;
        grandeTrame[SortiesN + 8 + (j++)] = ValidValues[(writedatabuffer[0]>>0) & 0x03];
        grandeTrame[SortiesN + 8 + (j++)] = ValidValues[(writedatabuffer[0]>>2) & 0x03];

        grandeTrame[SortiesN + 8 + (j++)] = (writedatabuffer[0]>>4) & 0x01;
        grandeTrame[SortiesN + 8 + (j++)] = (writedatabuffer[0]>>5) & 0x01;
        grandeTrame[SortiesN + 8 + (j++)] = (writedatabuffer[0]>>6) & 0x01;

    }
    if(status == SUSI_STATUS_WRITE_ERROR){
        printf("An error has occurred while writing data...\n");
        succeswriting = 0;
    }
    if(succeswriting){
        *paquet++;
        if (*paquet == 4294967295){
            *paquet = 0;
            for(i = PAQUET; i < MODE; i ++){
                grandeTrame[i] = 0;
            }
        }
        i = MODE - 1;
        if(grandeTrame[i] == 0){
            grandeTrame[i] = 1;
        } else{
            while(grandeTrame[i] != 0 && i>=PAQUET ){
                i--;
            }
            grandeTrame[i] = 1;
            i++;
            while(i < MODE){
                grandeTrame[i] = 0;
                i ++;
            }
        }
    }
    return succeswriting;
}

int readGPIO(unsigned char *grandeTrame){
    int j;
    int succesreading = 1;
    SusiStatus_t status;
    struct I2CConf config;
    uint8_t readdatabuffer[0x100]; //Input buffer
    readdatabuffer[0] = 0x00;
    int ValidValues[4] = {3,-1,0,1}; //{3d_State, ERROR, LOW ,HIGH}
    SusiId_t id = 0;
    int counter = 0;
    status = SusiGPIOSetLevel(BalyoGPIO_OutputEnable, GPIO_OutputMask, HIGH);
    if( status == SUSI_STATUS_SUCCESS ) counter++;
    /**********************************************/
    /******************Read GPI *******************/
    /**********************************************/
    printf("\n**********READING INPUTS*********\n\n");
    config.addr = GPI_Addr; //GPIO Input Address -voir BalyoLowLevel.h-
    config.rlen = 1; //1 octet a lire
    //****  Read Bank0 ****//
    config.cmd = GPI_0;
    status = SusiI2CReadTransfer(id, config.addr, config.cmd, readdatabuffer, config.rlen);
    if( status == SUSI_STATUS_SUCCESS ) counter++;
    if(counter == 2){
        printf("SUCCES Lecture on BANK 0\n");
        printf("Received data: ");
        printf("0x%02X\n", readdatabuffer[0]);
        j = 0;
        grandeTrame[EntreesN + (j++)] = ValidValues[(readdatabuffer[0]>>0) & 0x03];
        grandeTrame[EntreesN + (j++)] = ValidValues[(readdatabuffer[0]>>2) & 0x03];
        grandeTrame[EntreesN + (j++)] = ValidValues[(readdatabuffer[0]>>4) & 0x03];
        grandeTrame[EntreesN + (j++)] = ValidValues[(readdatabuffer[0]>>6) & 0x03];
    } else{
        printf("**************WARNING****************\n");
        printf("Impossible to read the data on current BANK \n");
        succesreading = 0;
    }
    //****** Read Bank 1 ****//
    config.cmd = GPI_1;
    status = SusiI2CReadTransfer(id, config.addr, config.cmd, readdatabuffer, config.rlen);
    if(status == SUSI_STATUS_SUCCESS){
        printf("SUCCES lecture on BANK 1\n");
        printf("Received data: ");
        printf("0x%02X\n", readdatabuffer[0]);
        j = 0;
        grandeTrame[EntreesN + 4 + (j++)] = ValidValues[(readdatabuffer[0]>>0) & 0x03];
        grandeTrame[EntreesN + 4 + (j++)] = ValidValues[(readdatabuffer[0]>>2) & 0x03];
        grandeTrame[EntreesN + 4 + (j++)] = ValidValues[(readdatabuffer[0]>>4) & 0x03];
        grandeTrame[EntreesN + 4 + (j++)] = ValidValues[(readdatabuffer[0]>>6) & 0x03];
    } else{
        printf("**************WARNING****************\n");
        printf("Impossible to read the data on current BANK \n");
    }
    //****** Read Bank 2 ****//
    config.cmd = GPI_2;
    status = SusiI2CReadTransfer(id, config.addr, config.cmd, readdatabuffer, config.rlen);
    if(status == SUSI_STATUS_SUCCESS){
        printf("SUCCES lecture on  BANK 2\n");
        printf("Received data: ");
        printf("0x%02X\n", readdatabuffer[0]);
        j = 0;
        grandeTrame[EntreesN + 8 + (j++)] = ValidValues[(readdatabuffer[0]>>0) & 0x03];
        grandeTrame[EntreesN + 8 + (j++)] = ValidValues[(readdatabuffer[0]>>2) & 0x03];
        if(readdatabuffer[0] & 0x10) grandeTrame[EntreesN + 8 + (j++)] = 1;
        else grandeTrame[EntreesN + 8 + (j++)] = 0;
        if(readdatabuffer[0] & 0x20) grandeTrame[EntreesN + 8 + (j++)] = 1;
        else grandeTrame[EntreesN + 8 + (j++)] = 0;
        if(readdatabuffer[0] & 0x40) grandeTrame[EntreesN + 8 + (j++)] = 1;
        else grandeTrame[EntreesN + 8 + (j++)] = 0;
        if(readdatabuffer[0] & 0x80) grandeTrame[EntreesN + 8 + (j++)] = 1;
        else grandeTrame[EntreesN + 8 + (j++)] = 0;
    } else{
        printf("**************WARNING****************\n");
        printf("Impossible to read the data on current BANK \n");
    }
    readdatabuffer[0] = 0xD0;
    status = SusiI2CWriteTransfer(id, config.addr, config.cmd, writedatabuffer, config.wlen);
    //****** Read Bank 3 ****//
    config.cmd = GPI_3;
    status = SusiI2CReadTransfer(id, config.addr, config.cmd, readdatabuffer, config.rlen);
    if(status == SUSI_STATUS_SUCCESS){
        printf("SUCCES lecture on BANK 3\n");
        printf("Received data: ");
        printf("0x%02X\n", readdatabuffer[0]);
        j = 0;
        if(readdatabuffer[0] & 0x01) grandeTrame[EntreesN + 14 + (j++)] = 1;
        else grandeTrame[EntreesN + 14 + (j++)] = 0;
        if(readdatabuffer[0] & 0x02) grandeTrame[EntreesN + 14 + (j++)] = 1;
        else grandeTrame[EntreesN + 14 + (j++)] = 0;
        if(readdatabuffer[0] & 0x04) grandeTrame[EntreesN + 14 + (j++)] = 1;
        else grandeTrame[EntreesN + 14 + (j++)] = 0;
        if(readdatabuffer[0] & 0x08) grandeTrame[EntreesN + 14 + (j++)] = 1;
        else grandeTrame[EntreesN + 14 + (j++)] = 0;
        if(readdatabuffer[0] & 0x10) grandeTrame[EntreesN + 14 + (j++)] = 1;
        else grandeTrame[EntreesN + 14 + (j++)] = 0;
        if(readdatabuffer[0] & 0x20) grandeTrame[EntreesN + 14 + (j++)] = 1;
        else grandeTrame[EntreesN + 14 + (j++)] = 0;
    } else{
        printf("**************WARNING****************\n");
        printf("Impossible to read the data on current BANK \n");
    }
    readdatabuffer[0] = 0xFF;
    status = SusiI2CWriteTransfer(id, config.addr, config.cmd, writedatabuffer, config.wlen);
    /**********************************************/
    /**************Read GPO ***********************/
    /**********************************************/
    printf("\n*********READING OUTPUTS*********\n\n");
    config.addr = GPO_Addr;
    config.cmd = GPO_0;
    status = SusiI2CReadTransfer(id, config.addr, config.cmd, readdatabuffer, config.rlen);
    if(status == SUSI_STATUS_SUCCESS){
        printf("SUCCES lecture on BANK 0\n");
        printf("Received data: ");
        printf("0x%02X\n", readdatabuffer[0]);
    } else{
        printf("**************WARNING****************\n");
        printf("Impossible to read the data on current BANK \n");
    }
    //****** Read Bank 1 ****//
    config.cmd = GPO_1;
    status = SusiI2CReadTransfer(id, config.addr, config.cmd, readdatabuffer, config.rlen);
    if(status == SUSI_STATUS_SUCCESS){
        printf("SUCCES lecture on BANK 1\n");
        printf("Received data: ");
        printf("0x%02X\n", readdatabuffer[0]);
    } else{
        printf("**************WARNING****************\n");
        printf("Impossible to read the data on current BANK \n");
    }
    config.cmd = GPO_2;
    status = SusiI2CReadTransfer(id, config.addr, config.cmd, readdatabuffer, config.rlen);
    if(status == SUSI_STATUS_SUCCESS){
        printf("SUCCES lecture on BANK 2\n");
        printf("Received data: ");
        printf("0x%02X\n", readdatabuffer[0]);
    } else{
        printf("**************WARNING****************\n");
        printf("Impossible to read the data on current BANK \n");
    }
    status = SusiGPIOSetLevel(BalyoGPIO_OutputEnable, GPIO_OutputMask, LOW);
    if( status != SUSI_STATUS_SUCCESS ) succesreading *= 0;
    return succesreading;
}


void leaveSUSI(){
    SusiId_t id = 0;
    int counter = 0;
    SusiStatus_t status;
    printf("Leaving SUSI... \n");
    status = SusiGPIOSetLevel(BalyoGPIO_OutputEnable, GPIO_OutputMask, HIGH);
    if(status == SUSI_STATUS_SUCCESS) counter++;
    status = SusiGPIOSetLevel(id, 0xFF, LOW);
    if(status == SUSI_STATUS_SUCCESS) counter++;
    status = SusiLibUninitialize();
    if(status == SUSI_STATUS_SUCCESS) counter++;
    sleep(0.5);
    if(counter == 3){
        printf("*****************************************************************************************************\n");
        printf("                                              SUSI out                                               \n");
        printf("*****************************************************************************************************\n");
    }
}


void systemInformation(){
    SusiStatus_t status;
    uint32_t pValue;
    printf("\n*****************************************************************************************************\n");
    printf("                                       [SUSI -] System information:                             \n");
    printf("*****************************************************************************************************\n");
    status = SusiBoardGetValue(SUSI_ID_GET_SPEC_VERSION, &pValue);
    if(status == SUSI_STATUS_SUCCESS) printf("API version: %d\n", pValue);
    status = SusiBoardGetValue(SUSI_ID_BOARD_DRIVER_VERSION_VAL, &pValue);
    if(status == SUSI_STATUS_SUCCESS) printf("DRIVER version: %d\n", pValue);
    status = SusiBoardGetValue(SUSI_ID_HWM_VOLTAGE_VCORE, &pValue);
    if(status == SUSI_STATUS_SUCCESS) printf("CPU Core voltage: %d mV\n", pValue);
    status = SusiBoardGetValue(SUSI_ID_HWM_VOLTAGE_VCORE2, &pValue);
    if(status == SUSI_STATUS_SUCCESS) printf("Second CPU Core voltage: %d mV\n", pValue);
    status = SusiBoardGetValue(SUSI_ID_HWM_TEMP_CPU, &pValue);
    if(status == SUSI_STATUS_SUCCESS) printf("CPU temperature: %de-1 K\n", pValue);
    status = SusiBoardGetValue(SUSI_ID_HWM_TEMP_CPU2, &pValue);
    if(status == SUSI_STATUS_SUCCESS) printf("CPU2 temperature: %de-1 K\n", pValue);
    status = SusiBoardGetValue(SUSI_ID_HWM_TEMP_SYSTEM, &pValue);
    if(status == SUSI_STATUS_SUCCESS) printf("System temperature: %de-1 K\n", pValue);
    status = SusiBoardGetValue(SUSI_ID_HWM_FAN_CPU, &pValue);
    if(status == SUSI_STATUS_SUCCESS) printf("CPU fan speed: %d RPM\n", pValue);
    status = SusiBoardGetValue(SUSI_ID_HWM_FAN_CPU2, &pValue);
    if(status == SUSI_STATUS_SUCCESS) printf("CPU2 fan speed: %d RPM\n", pValue);
    status = SusiBoardGetValue(SUSI_ID_HWM_FAN_SYSTEM, &pValue);
    if(status == SUSI_STATUS_SUCCESS) printf("System fan speed: %d RPM\n", pValue);
    status = SusiBoardGetValue(SUSI_ID_HWM_TEMP_CPU, &pValue);
    if(status == SUSI_STATUS_SUCCESS) printf("CPU temperature: %de-1 K\n", pValue);
    status = SusiBoardGetValue(SUSI_ID_HWM_VOLTAGE_2V5, &pValue);
    if(status == SUSI_STATUS_SUCCESS) printf("Voltage en 2V5: %d mV\n", pValue);
    status = SusiBoardGetValue(SUSI_ID_HWM_VOLTAGE_3V3, &pValue);
    if(status == SUSI_STATUS_SUCCESS) printf("Voltage en 3V3: %d mV\n", pValue);
    status = SusiBoardGetValue(SUSI_ID_HWM_VOLTAGE_5V, &pValue);
    if(status == SUSI_STATUS_SUCCESS) printf("Voltage en 5V: %d mV\n", pValue);
    status = SusiBoardGetValue(SUSI_ID_HWM_VOLTAGE_12V, &pValue);
    if(status == SUSI_STATUS_SUCCESS) printf("Voltage en 12V: %d mV\n", pValue);
    status = SusiBoardGetValue(SUSI_ID_HWM_VOLTAGE_5VSB, &pValue);
    if(status == SUSI_STATUS_SUCCESS) printf("Voltage en 5V Standby: %d mV\n", pValue);
    status = SusiBoardGetValue(SUSI_ID_HWM_VOLTAGE_3VSB, &pValue);
    if(status == SUSI_STATUS_SUCCESS) printf("Voltage en 3V Standby: %d mV\n", pValue);
    status = SusiBoardGetValue(SUSI_ID_HWM_VOLTAGE_VBAT, &pValue);
    if(status == SUSI_STATUS_SUCCESS) printf("Voltage en CMOS Battery voltage: %d mV\n", pValue);
    status = SusiBoardGetValue(SUSI_ID_HWM_VOLTAGE_VTT, &pValue);
    if(status == SUSI_STATUS_SUCCESS) printf("Voltage en DIMM voltage: %d mV\n", pValue);
    status = SusiBoardGetValue(SUSI_ID_HWM_VOLTAGE_24V, &pValue);
    if(status == SUSI_STATUS_SUCCESS) printf("Voltage en 24V: %d mV\n", pValue);
    printf("*****************************************************************************************************\n");
    printf("...\n...\n...\nPress enter to continue\n");
    getchar();
}

void I2CFrequency(){
    SusiStatus_t status;
    uint32_t ID = 0;
    uint32_t pFreq;
    status = SusiI2CGetFrequency(ID, &pFreq);
    if(status == SUSI_STATUS_SUCCESS) printf("Frequency on I2CBus: %d Hz\n\n", pFreq);
}



int menu(){

    int choice;
    printf("*****************************************************************************************************\n");
    printf("                                                 MENU                                         \n");
    printf("*****************************************************************************************************\n");
    printf("Please make a choice:\n\n");
    printf("1) Activate OUTPUTS.\n");
    printf("2) Deactivate OUTPUTS.\n");
    printf("3) OUTPUTS to 3rd state.\n");
    printf("4) Select each output individually.\n");
    printf("5) Update I/O from external frame\n");
    printf("6) Show system information.\n");
    printf("7) Leave SUSI.\n\n");
    printf("Your choice: ");
    scanf("%d", &choice);
    return choice;
}


uint8_t buffer2write(int choice, int bank, unsigned char *trameRecu){
    uint8_t buffer[0x100];
    buffer[0] = 0x00;
    int activation;
    int i;
    switch(choice){
        case 1:
                switch(bank){
                    case 0:
                            buffer[0] = 0x55;
                            break;
                    case 1:
                            buffer[0] = 0x55;
                            break;
                    case 2:
                            buffer[0] = 0x75;
                            break;
                }
                break;
        case 2:
                switch(bank){
                    case 0:
                            buffer[0] = 0xAA;
                            break;
                    case 1:
                            buffer[0] = 0xAA;
                            break;
                    case 2:
                            buffer[0] = 0x0A;
                            break;
                }
                break;
        case 3:
                buffer[0] = 0x00;
                break;
        case 4:
                switch(bank){
                    case 0:
                            printf("Activate SN1? [y(1)/n(0)]: ");
                            scanf("%d", &activation);
                            if(activation == 1) buffer[0] = buffer[0] | 0x01;
                            else buffer[0] = buffer[0] | 0x02;
                            printf("Activate SN2? [y(1)/n(0)]: ");
                            scanf("%d", &activation);
                            if(activation == 1) buffer[0] = buffer[0] | 0x04;
                            else buffer[0] = buffer[0] | 0x08;
                            printf("Activate SN3? [y(1)/n(0)]: ");
                            scanf("%d", &activation);
                            if(activation == 1) buffer[0] = buffer[0] | 0x10;
                            else buffer[0] = buffer[0] | 0x20;
                            printf("Activate SN4? [y(1)/n(0)]: ");
                            scanf("%d", &activation);
                            if(activation == 1) buffer[0] = buffer[0] | 0x40;
                            else buffer[0] = buffer[0] | 0x80;
                            break;
                    case 1:
                            printf("Activate SN5? [y(1)/n(0)]: ");
                            scanf("%d", &activation);
                            if(activation == 1) buffer[0] = buffer[0] | 0x01;
                            else buffer[0] = buffer[0] | 0x02;
                            printf("Activate SN6? [y(1)/n(0)]: ");
                            scanf("%d", &activation);
                            if(activation == 1) buffer[0] = buffer[0] | 0x04;
                            else buffer[0] = buffer[0] | 0x08;
                            printf("Activate SN7? [y(1)/n(0)]: ");
                            scanf("%d", &activation);
                            if(activation == 1) buffer[0] = buffer[0] | 0x10;
                            else buffer[0] = buffer[0] | 0x20;
                            printf("Activate SN8? [y(1)/n(0)]: ");
                            scanf("%d", &activation);
                            if(activation == 1) buffer[0] = buffer[0] | 0x40;
                            else buffer[0] = buffer[0] | 0x80;
                            break;
                    case 2:
                            printf("Activate SN9? [y(1)/n(0)]: ");
                            scanf("%d", &activation);
                            if(activation == 1) buffer[0] = buffer[0] | 0x01;
                            else buffer[0] = buffer[0] | 0x02;
                            printf("Activate SN10? [y(1)/n(0)]: ");
                            scanf("%d", &activation);
                            if(activation == 1) buffer[0] = buffer[0] | 0x04;
                            else buffer[0] = buffer[0] | 0x08;
                            printf("Activate XsManu1? [y(1)/n(0)]: ");
                            scanf("%d", &activation);
                            if(activation == 1) buffer[0] = buffer[0] | 0x10;
                            else buffer[0] = buffer[0] | 0x00;
                            printf("Activate XsManu2? [y(1)/n(0)]: ");
                            scanf("%d", &activation);
                            if(activation == 1) buffer[0] = buffer[0] | 0x20;
                            else buffer[0] = buffer[0] | 0x00;
                            printf("Activate XsGo? [y(1)/n(0)]: ");
                            scanf("%d", &activation);
                            if(activation == 1) buffer[0] = buffer[0] | 0x40;
                            else buffer[0] = buffer[0] | 0x00;
                            break;
                }
                break;
        case 5:
                i = 0;
                switch(bank){
                    case 0:
                            if(trameRecu[i++]) buffer[0] = buffer[0] | 0x01;
                            else buffer[0] = buffer[0] | 0x02;
                            if(trameRecu[i++]) buffer[0] = buffer[0] | 0x04;
                            else buffer[0] = buffer[0] | 0x08;
                            if(trameRecu[i++]) buffer[0] = buffer[0] | 0x10;
                            else buffer[0] = buffer[0] | 0x20;
                            if(trameRecu[i++]) buffer[0] = buffer[0] | 0x40;
                            else buffer[0] = buffer[0] | 0x80;
                            break;
                    case 1:
                            if(trameRecu[ 4 + i++]) buffer[0] = buffer[0] | 0x01;
                            else buffer[0] = buffer[0] | 0x02;
                            if(trameRecu[ 4 + i++]) buffer[0] = buffer[0] | 0x04;
                            else buffer[0] = buffer[0] | 0x08;
                            if(trameRecu[ 4 + i++]) buffer[0] = buffer[0] | 0x10;
                            else buffer[0] = buffer[0] | 0x20;
                            if(trameRecu[ 4 + i++]) buffer[0] = buffer[0] | 0x40;
                            else buffer[0] = buffer[0] | 0x80;
                            break;
                    case 2:
                            if(trameRecu[ 8 + i++]) buffer[0] = buffer[0] | 0x01;
                            else buffer[0] = buffer[0] | 0x02;
                            if(trameRecu[ 8 + i++]) buffer[0] = buffer[0] | 0x04;
                            else buffer[0] = buffer[0] | 0x08;
                            if(trameRecu[ 8 + i++]) buffer[0] = buffer[0] | 0x10;
                            else buffer[0] = buffer[0] | 0x00;
                            if(trameRecu[ 8 + i++]) buffer[0] = buffer[0] | 0x20;
                            else buffer[0] = buffer[0] | 0x00;
                            if(trameRecu[ 8 + i++]) buffer[0] = buffer[0] | 0x40;
                            else buffer[0] = buffer[0] | 0x00;
                            break;
                }
                break;
            case 6:
                switch(bank){
                    case 0:
                            buffer[0] = 0x00;
                            break;
                    case 1:
                            buffer[0] = 0x00;
                            break;
                    case 2:
                            buffer[0] = 0x10;
                            break;
                }
                break;
            case 7:
                switch(bank){
                    case 0:
                            buffer[0] = 0x00;
                            break;
                    case 1:
                            buffer[0] = 0x00;

                            break;
                    case 2:
                            buffer[0] = 0x20;
                            break;
                }
                break;
            case 8:
                switch(bank){
                    case 0:
                            buffer[0] = 0x00;
                            break;
                    case 1:
                            buffer[0] = 0x00;
                            break;
                    case 2:
                            buffer[0] = 0x40;
                            break;
                }
                break;
            case 9:
                switch(bank){
                    case 0:
                            buffer[0] = 0x00;
                            break;
                    case 1:
                            buffer[0] = 0x00;
                            break;
                    case 2:
                            buffer[0] = 0x30;
                            break;
                }
                break;
            case 10:
                switch(bank){
                    case 0:
                            buffer[0] = 0x00;
                            break;
                    case 1:
                            buffer[0] = 0x00;
                            break;
                    case 2:
                            buffer[0] = 0x60;
                            break;
                }
                break;
            case 11:
                switch(bank){
                    case 0:
                            buffer[0] = 0x00;
                            break;
                    case 1:
                            buffer[0] = 0x00;
                            break;
                    case 2:
                            buffer[0] = 0x50;
                            break;
                }
                break;
            case 12:
                switch(bank){
                    case 0:
                            buffer[0] = 0x00;
                            break;
                    case 1:
                            buffer[0] = 0x00;
                            break;
                    case 2:
                            buffer[0] = 0x70;
                            break;
                }
                break;
    }
    return buffer[0];
}


void FakeTrame(unsigned char *fakeTrame){
    int i;
    for(i = 0; i < NumberOfBits; i++){
        fakeTrame[i] = 0;
    }
    i = 0;
    /** En-tête **/
    /** Start **/
    fakeTrame[start + i++ ] = 1;
    fakeTrame[start + i++ ] = 0;
    fakeTrame[start + i++ ] = 0;
    fakeTrame[start + i++ ] = 0;
    fakeTrame[start + i++ ] = 0;
    fakeTrame[start + i++ ] = 0;
    fakeTrame[start + i++ ] = 0;
    fakeTrame[start + i++ ] = 1;
    i = 0;
    /** Type **/
    fakeTrame[type + i++ ] = 0;
    fakeTrame[type + i++ ] = 0;
    fakeTrame[type + i++ ] = 0;
    fakeTrame[type + i++ ] = 0;
    fakeTrame[type + i++ ] = 0;
    fakeTrame[type + i++ ] = 0;
    fakeTrame[type + i++ ] = 0;
    fakeTrame[type + i++ ] = 0;
    i = 0;
    /** Size **/
    dec2bin(&fakeTrame[0], CRC, NumberOfBits);
    /** CRC **/

    /** No paquet **/

    /** Mode **/
    fakeTrame[version - 1] = 1;
    /** Version **/
    fakeTrame[SortiesN - 1] = 1;
    /** Sorties **/
    i = 0;
    //  ACTIVATION DE SN1 ET SN3
    fakeTrame[SortiesN + i++] = 0;
    fakeTrame[SortiesN + i++] = 1;
    fakeTrame[SortiesN + i++] = 1;
    fakeTrame[SortiesN + i++] = 0;
    fakeTrame[SortiesN + i++] = 0;
    fakeTrame[SortiesN + i++] = 0;
    fakeTrame[SortiesN + i++] = 0;
    fakeTrame[SortiesN + i++] = 0;
    fakeTrame[SortiesN + i++] = 0;
    fakeTrame[SortiesN + i++] = 0;
    fakeTrame[SortiesN + i++] = 0;
    fakeTrame[SortiesN + i++] = 0;
    fakeTrame[SortiesN + i++] = 0;
    fakeTrame[SortiesN + i++] = 0;
    fakeTrame[SortiesN + i++] = 0;
    fakeTrame[SortiesN + i++] = 0;
    /** Entrees **/
    i = 0;
    fakeTrame[EntreesN + i++] = 0;
    fakeTrame[EntreesN + i++] = 0;
    fakeTrame[EntreesN + i++] = 0;
    fakeTrame[EntreesN + i++] = 0;
    fakeTrame[EntreesN + i++] = 0;
    fakeTrame[EntreesN + i++] = 0;
    fakeTrame[EntreesN + i++] = 0;
    fakeTrame[EntreesN + i++] = 0;
    fakeTrame[EntreesN + i++] = 0;
    fakeTrame[EntreesN + i++] = 0;
    fakeTrame[EntreesN + i++] = 0;
    fakeTrame[EntreesN + i++] = 0;
    fakeTrame[EntreesN + i++] = 0;
    fakeTrame[EntreesN + i++] = 0;
    fakeTrame[EntreesN + i++] = 0;
    fakeTrame[EntreesN + i++] = 0;
    fakeTrame[EntreesN + i++] = 0;
    fakeTrame[EntreesN + i++] = 0;
    fakeTrame[EntreesN + i++] = 0;
    fakeTrame[EntreesN + i++] = 0;
}

void crcCalcul(unsigned char *grandeTrame){
    uint32_t crc;
    int k = 0;
    crc = crc32(0, &grandeTrame[0], 32);
    dec2bin(&grandeTrame[0], PAQUET, (int)crc);
}

void miseAJourEnteteTrame(unsigned char *grandeTrame){
    dec2bin(&grandeTrame[0], type, 129);
    dec2bin(&grandeTrame[0], CRC, NumberOfBits); // Ecriture du nombre des données dans la trame
    dec2bin(&grandeTrame[0], SortiesN, 1); // Ecriture de la version
    crcCalcul(&grandeTrame[0]); // Calcul du CRC
}
