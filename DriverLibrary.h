
// Structure allowing to change the configuration of the GPIO according to each BANK
#define NumberOfBanks 5

//**** Config Advantch *******//
static SusiId_t devids[SUSI_I2C_MAX_DEVICE]; //

//******** GPIO Advantech *********//
#define BalyoGPIO_Reset SUSI_ID_GPIO(0)
#define BalyoGPIO_OutputEnable SUSI_ID_GPIO(1)
#define BalyoWatchDog_L1 2
#define BalyoWatchDog_L2 3
#define GPIO_OutputMask 1
#define HIGH 1
#define LOW  0

//********* WATCHDOG TIMMING ************//
#define T_Watchdog = 333000 // T = 1/F  ou T[µs] et F[Hz]


//********* GPIO_BALYO  -I2C- DEFINITION S ************//

//Adresses des circuits I2C (voir doc IC PCA9505 pour les pines A0 A1 et A2
//Dans le diagramme du circuit electrique, pour les entress, A0=A1=A2=LOW. Pour les sorties A0=HIGH, A1=A2=LOW
//Les adresses son 0x4?h à cause du shifting des adresses I2C gere par la carte Advantech (SUSI)

#define GPI_Addr 0x40  //Adresse de lecture (entrees)
#define GPO_Addr 0x42  //Adresse d'ecriture (sorties)

// Input Port Registers (cf. doc IC PCA9505)
#define GPI_0 0x00
#define GPI_1 0x01
#define GPI_2 0x02
#define GPI_3 0x03
#define GPI_4 0x04

// Output Port Registers (cf. doc IC PCA9505)
#define GPO_0 0x08
#define GPO_1 0x09
#define GPO_2 0x0A
#define GPO_3 0x0B
#define GPO_4 0x0C

// I/O Configuration registers (cf. doc IC PCA9505)
#define GPIO_Conf_0 0x18
#define GPIO_Conf_1 0x19
#define GPIO_Conf_2 0x1A
#define GPIO_Conf_3 0x1B
#define GPIO_Conf_4 0x1C

// Bits addresses definitions

#define NumberOfBits 160

#define start 0
#define type 8
#define size 16
#define CRC 32
#define PAQUET 64
#define MODE 96
#define version 104
#define SortiesN 120
#define EntreesN 136



//*Definition de l'estructure pour ecrire/lire le bus I2C
struct I2CConf{
	//enum addrtypeRank addrType;
	uint32_t addr;
	//enum cmdtypeRank cmdType;
	uint32_t cmd;
	uint8_t *wdata;
	uint32_t wlen;
	uint8_t *rdata;
	uint32_t rlen;

};



static const uint8_t I2C_Bank_Configuration[NumberOfBanks] =
{
    GPIO_Conf_0,
    GPIO_Conf_1,
    GPIO_Conf_2,
    GPIO_Conf_3,
    GPIO_Conf_4,
};

char binary2hex(unsigned char *valeur);

int iic_probe(uint8_t iDevice, int addrType) ;

int initializationDriver(unsigned char *grandeTrame);

int selectionMode(unsigned char *grandeTrame);

int confGPIO();

int writeGPIO(unsigned char *grandeTrame, unsigned int *paquet, int choice, unsigned char *trameRecu);

int readGPIO(unsigned char *grandeTrame);

void printTrameBinary(unsigned char *grandeTrame);

void printTrameHex(unsigned char *grandeTrame);

void leaveSUSI();

void systemInformation();

void I2CFrequency();

void clearTrame(unsigned char *Trame);

int menu();


uint8_t buffer2write(int choice, int bank, unsigned char *trameRecu);

void dec2bin(unsigned char *grandeTrame, int adresseSuivante, int nombre);

void FakeTrame(unsigned char *fakeTrame);

void crcCalcul(unsigned char *grandeTrame);

void miseAJourEnteteTrame(unsigned char *grandeTrame);
