/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2011 - www.soldersplash.co.uk - C. Matthews - R. Steel

 This work is licensed under a Creative Commons Attribution-ShareAlike 3.0 Unported License.

*/

#define SSC_UDP_PORT_RX		11028
#define SSC_UDP_PORT_TX		11029

#define SSC_REPLY_LEN			118
#define SSC_EXTD_REPLY_LEN		66
#define SSC_INFO_MSG_LEN		99



#ifdef UDPCONTROL

static ui8 SscReplyBuffer[SSC_REPLY_LEN];

enum SSC_COMMANDS 
{
	SSC_PING = 1,						// Check the Unit is there
	SSC_EXTENDED_PING,					// Get Extended Info
	SSC_RELAY_CON = 10,					// Control it's relays
	SSC_PWM_DUTY,						// Duty for a single PWM
	SSC_PWM_DUTY_ALL,					// Duty for a PWMs
	SSC_PWM_FREQ,						// Set Freq of the PWMs masked
	SSC_PWM_COLOUR_MODE,				// Use a colour mode to control the PWMs
	SSC_SET_UNIT_NAME = 0x20,
	SSC_SET_RELAY_NAME,					// Set the supplied relay no's name
	SSC_OUTPUTS_ON_OFF = 0x30,			// Control all outputs, turn if on/off
	SSC_SPLASHPIXEL_FBSET = 0x45,		// Set the SplashPixel Framebuffer
	SSC_MANUAL_GPIO_DIR = 0x50, 		// Set gpio direction, 1 output
	SSC_MANUAL_GPIO_DATA = 0x51, 		// set gpio outputs high or low
	SSC_LOGIC_COMMAND = 0x60,
	SSC_LOGIC_INSERT_CON = 0x61,		// Insert a command
	SSC_BRIDGE_SCAN = 0x80,
	SSC_SB_SERVOPOS = 0x90,

	SSC_RESET = 0xFF
};

enum SSC_REPLY_POSITIONS
{
	SSC_POS_STARTBYTE = 0,
	SSC_POS_IP,
	SSC_POS_MAC = 5,
	SSC_POS_SWREV = 11,
	SSC_POS_UNITNAME = 13,
	SSC_POS_CONFIG_BITS = 36,
	SSC_POS_SPARE,
	SSC_POS_ADC0,
	SSC_POS_ADC1 = 40,
	SSC_POS_ADC2 = 42,
	SSC_POS_RELAYSTATE = 44,
	SSC_POS_PWM0DUTY,
	SSC_POS_PWM1DUTY = 47,
	SSC_POS_PWM2DUTY = 49,
	SSC_POS_PWMFREQ = 51,
	SSC_POS_COLOURMODE = 53,

	SSC_POS_PWM3DUTY = 54,
	SSC_POS_PWM4DUTY = 56,
	SSC_POS_PWMFREQ2 = 58,

	SSC_POS_REDDUTY = 60,
	SSC_POS_GREENDUTY,
	SSC_POS_BLUEDUTY,

	SSC_POS_STEPSIZE,
	SSC_POS_STEPCNT,

	SSC_POS_RELAY1NAME = 66,
	SSC_POS_RELAY2NAME = 78,
	SSC_POS_RELAY3NAME = 90,
	SSC_POS_RELAY4NAME = 102,

	SSC_POS_TIMESTAMP = 114
};


// A simple packet structure, command byte and data
// UDP layer handles checksuming, all data is to be in a single packet, no fragmentation!
struct SSC_COMMAND_STRUCT 
{
	ui8 command;
	ui8 data[50];
	ui8 length;
} ssc_command;



struct udp_pcb *UdpControlPort;

static void SSC_ProcessCommand(struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr);
static void SSC_SendReply(struct ip_addr *addr);
static void SSC_Recieve(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, ui16 port);


#endif /*UDPCONTROL_H_*/

void SSC_Init(void);
void SSC_MACAddrSet(ui8 *pucMACArray);
void SSC_SetRelayNames ( void );
void SSC_SetUnitName(const ui8 *pcAppTitle);
void SSC_SendPortInfo ( struct ip_addr *addr, ui16 reason );
void SSC_UpdatePwmDuty( ui8 *buffer );
void SSC_SplashPixelUpdate ( struct pbuf *p );

