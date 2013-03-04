/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2013-03-03 - www.soldersplash.co.uk - C. Matthews - R. Steel

 This work is licensed under a Creative Commons Attribution-ShareAlike 3.0 Unported License.

*/

// Max String Lengths
#define SPLASHBASE_NAME_LEN				24
#define SPLASHBASE_RELAYNAME_LEN		12
#define COSM_PRIV_KEY_LEN				64
#define SNTP_SERVER_LEN					32

// Structure of configuration saved to flash
typedef struct
{
	// TI Flash library wear levels config writes to the range of flash used.
	// the config block with the highest sequence num is loaded is loaded/used
    ui8 sequenceNum;

    // TI Flash writing library stores and checks this checksum
    ui8 crcChecksum;

    // Version of config
    ui8 version;

    // Misc Flags
    ui8 flags;

    // SplashBase Name
    ui8 splashBaseName[SPLASHBASE_NAME_LEN];

    // The static IP address to use if DHCP is not in use
    ui32 ulStaticIP;

    // The default gateway IP address to use if DHCP is not in use.
    ui32 ulGatewayIP;

    // The subnet mask to use if DHCP is not in use.
    ui32 ulSubnetMask;

    // Relay Names
    ui8 relayOneName[SPLASHBASE_RELAYNAME_LEN];
    ui8 relayTwoName[SPLASHBASE_RELAYNAME_LEN];
    ui8 relayThreeName[SPLASHBASE_RELAYNAME_LEN];
    ui8 relayFourName[SPLASHBASE_RELAYNAME_LEN];

    ui16 pwmFreq;
    ui8 colourMode;
    ui8 colourStepSize;

    ui16 colourSteps;
    ui16 reserved;

    ui32 colour1;
    ui32 colour2;

    // Time Outset in minutes
    si32 timeOffset;

    // SNTP ServerAddress
    ui8 sntpServerAddress[SNTP_SERVER_LEN];

    // COSM Private key
    ui8 cosmPrivKey[COSM_PRIV_KEY_LEN];

    // 120 bytes to hold the direction and pin high/low initialisation
    ui32 UserGpioInit[15][2];

    // 20x28 = 560 Bytes
    ui8 LogicConditionsBuffer[ 560 ];
    //LOGIC_CONDITION LogicConditions[LOGIC_MAX_CONDITIONS];

    // Padding to ensure the whole structure is 1024 bytes long.
    ui8 ucReserved2[137];
}
tConfigParameters;

//*****************************************************************************
//
//! If this flag is set in the ucFlags field of tConfigParameters, the module
//! uses a static IP.  If not, DHCP and AutoIP are used to obtain an IP
//! address.
//
//*****************************************************************************
#define CONFIG_FLAG_STATICIP    	0x80
#define CONFIG_USE_CUSTOM_SNTP    	0x40

//*****************************************************************************
//
//! The address of the first block of flash to be used for storing parameters.
//
//*****************************************************************************
#define FLASH_PB_START          0x00017800

//*****************************************************************************
//
//! The address of the last block of flash to be used for storing parameters.
//! Since the end of flash is used for parameters, this is actually the first
//! address past the end of flash.
//
//*****************************************************************************
#define FLASH_PB_END            0x00018000

//*****************************************************************************
//
//! The size of the parameter block to save.  This must be a power of 2,
//! and should be large enough to contain the tConfigParameters structure.
//
//*****************************************************************************
#define FLASH_PB_SIZE           1024


//*****************************************************************************
//
// Prototypes for the globals exported from the configuration module, along
// with public API function prototypes.
//
//*****************************************************************************
extern tConfigParameters g_sParameters;
extern tConfigParameters g_sWorkingDefaultParameters;

extern const tConfigParameters *g_psDefaultParameters;
extern const tConfigParameters *const g_psFactoryParameters;
extern void ConfigInit(void);
extern void ConfigLoadFactory(void);
extern void ConfigLoad(void);
extern void ConfigSave(void);
extern void ConfigWebInit(void);
extern void ConfigUpdateIPAddress(void);
extern void ConfigUpdateAllParameters(tBoolean bUpdateIP);

