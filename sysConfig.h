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
#define COSM_API_KEY_LEN				64
#define COSM_HOST_LEN					32
#define COSM_URL_LEN					32
#define SNTP_SERVER_LEN					32


// Bit masks for the SystemConfig.flags value
#define CONFIG_FLAG_STATICIP    	0x80
#define CONFIG_USE_CUSTOM_SNTP    	0x40
#define CONFIG_USER_GPIO_INIT		0x20
#define CONFIG_FOUR_RELAY_EN		0x10
#define CONFIG_RGB_PWM_EN			0x08


typedef struct
{
	ui8 Spare2:1;
	ui8 Spare1:1;
	ui8 LogicEnabled:1;
	ui8 RgbPwmEnable:1;
	ui8 FourRelayEnable:1;
	ui8 UserGpioInit:1;
	ui8 CustomSNTP:1;
	ui8 StaticIp:1;
}
SysConfigFlags;

// SystemConfig Structure, used to hold all of the Non volatile configuration
// Written to the end of the flash
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
    //ui8 flags;
    SysConfigFlags flags;

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

    // 120 bytes to hold the direction and pin high/low initialisation
    ui32 UserGpioInit[15][2];

    // 20x28 = 560 Bytes
    //ui8 LogicConditionsBuffer[ 560 ];
    LOGIC_CONDITION LogicConditionsBuffer[LOGIC_MAX_CONDITIONS];

    // HTTP Private/API key
    ui8 cosmPrivKey[COSM_API_KEY_LEN];

    // HTTP host
    ui8 cosmHost[COSM_HOST_LEN];

    // HTTP URL (without the host)
    ui8 cosmUrl[COSM_URL_LEN];

    // Padding to ensure the whole structure is 1024 bytes long.
    ui8 ucReserved2[73];
}
tConfigParameters;

#ifdef _CONFIG_

static const tConfigParameters CONFIG_FACTORY_DEFAULTS =
{
    // The sequence number
    (ui8) 0,

    // Checksum
    (ui8) 0,

    // version
    (ui8) 0,

    // flags
    {0,0,0,1,1,1,0,0},

    // SplashBase Name
    {
		'S','p','l','a','s','h','B','a','s','e', 0 , 0 , 0 , 0 , 0 , 0,
		 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0,
	},

    // Static IP address
	(ui32)0x00000000,

    // Default gateway IP address (used only if static IP is in use).
	(ui32)0x00000000,

    // Subnet mask (used only if static IP is in use).
	(ui32)0xFFFFFF00,

    // Relay Names
    {
    	'R','e','l','a','y','1', 0, 0, 0, 0, 0, 0,
    },
    {
        'R','e','l','a','y','2', 0, 0, 0, 0, 0, 0,
    },
    {
        'R','e','l','a','y','3', 0, 0, 0, 0, 0, 0,
    },
    {
        'R','e','l','a','y','4', 0, 0, 0, 0, 0, 0,
    },

    // PWM freq = 10kHz
    (ui16)5000,

    (ui8)COLOUR_MODE_OFF,
    // Step length ( in ticks 10ms )
    (ui8)1,

    // No of steps between top and bottom
    (ui16)255,

    // Reserved to fill a whole
    (ui16)0,

    // Top Colour (Colour1)
    (ui32)0x00000000,

    // Bottom Colour (Colour2)
    (ui32)0x00000000,

    // Time offset
    (si32)0,

    // SNTP Address 0.pool.ntp.org
	{
		'0','.','p','o','o','l','.','n','t','p','.','o','r','g', 0 , 0 ,
		 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	},

    //
    // (compiler will pad to the full length)
    //
};

tConfigParameters SystemConfig;

#endif

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
extern tConfigParameters SystemConfig;


void SysConfigInit(void);
void SysConfigFactoryDefault(void);
void SysConfigLoad(void);
void SysConfigSave(void);

void SysSetBaseName ( ui8 *buffer, ui8 len );
void SysSetSntpAddress ( ui8 *buffer, ui8 len );
void SysConfigRelayEnable ( void );
void SysConfigRelayDisable ( void );
void SysSetRelayName ( ui8 *buffer, ui8 len, ui8 relayNo, bool saveNow );


