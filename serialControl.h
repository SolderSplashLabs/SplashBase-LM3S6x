/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2012 - www.soldersplash.co.uk - C. Matthews - R. Steel

*/


// Max no of arguments per command
#define CMDLINE_MAX_ARGS 5


// Port and Pin Defines used when enabling the UART
#define PIN_U0RX_PORT           GPIO_PORTA_BASE
#define PIN_U0RX_PIN            GPIO_PIN_0
#define PIN_U0TX_PORT           GPIO_PORTA_BASE
#define PIN_U0TX_PIN            GPIO_PIN_1

#define PIN_U1RX_PORT           GPIO_PORTD_BASE
#define PIN_U1RX_PIN            GPIO_PIN_2
#define PIN_U1TX_PORT           GPIO_PORTD_BASE
#define PIN_U1TX_PIN            GPIO_PIN_3


void Serial_Task ( void );
void Serial_Init ( void );
void Serial_PrintGpioDetails ( ui8 portNo );
void Serial_PrintBridgeList ( void );

// Functions called by the command processor
extern int CMD_help (int argc, char **argv);
extern int CMD_SetName (int argc, char **argv);
extern int CMD_NotImplemented (int argc, char **argv);
extern int CMD_Reboot (int argc, char **argv);
extern int CMD_Date (int argc, char **argv);
extern int CMD_Relay (int argc, char **argv);
extern int CMD_Adcs (int argc, char **argv);
extern int CMD_Factory (int argc, char **argv);
extern int CMD_Uptime (int argc, char **argv);
extern int CMD_ServoMove (int argc, char **argv);
extern int CMD_Rgb (int argc, char **argv);
extern int CMD_BridgeScan (int argc, char **argv);
extern int CMD_BridgeList (int argc, char **argv);
extern int CMD_rand (int argc, char **argv);
extern int CMD_intensity (int argc, char **argv);
extern int CMD_ipconfig (int argc, char **argv);
extern int CMD_GetTemperature (int argc, char **argv);
extern int CMD_GetLogic (int argc, char **argv);
extern int CMD_GpioInit (int argc, char **argv);
extern int CMD_Gpio (int argc, char **argv);
extern int CMD_SelfTest (int argc, char **argv);
extern int CMD_cosm (int argc, char **argv);

#ifdef _SERIAL_CONTROL_

// Serial Banner, this looks mangled but in putty it looks good!
const char WELCOME_MSG[] = "\n  ____        _     _           ____        _           _	 _          _\n\
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___ \n\
 \\___ \\ / _ \\| |/ _` |/ _ \\ '__\\___ \\| '_ \\| |/ _` / __| '_ \\	| |   / _` | '_ \\/ __| \n\
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \\__ \\ | | |	| |__| (_| | |_) \\__ \\ \n\
 |____/ \\___/|_|\\__,_|\\___|_|  |____/| .__/|_|\\__,_|___/_| |_|	|_____\\__,_|_.__/|___/ \n\
                                     |_| \nSolderSplash Labs - SplashBase V";

const char ACK_STRING[] = "Acknowledged\n";

// Input buffer for the command line interpreter.
static char g_cInput[200];

//*****************************************************************************
//
// Table of valid command strings, callback functions and help messages.
//
//*****************************************************************************
tCmdLineEntry g_sCmdTable[] =
{
    {"help",     	CMD_help,      			" : Display list of commands" },
    {"?",     		CMD_help,      			" : Display list of commands" },
    {"config",		CMD_Factory,			" : <save>/<factorydefault> save or factory default settings"},
    {"setname",  	CMD_SetName,     		" : name - Set SplashBase name"},
    {"ipconfig", 	CMD_ipconfig,  			" : <dynamic>/<static> <ip> <mask> <gateway> Show/Set Network Config"},
    {"cosm", 		CMD_cosm,  				" : <setkey>/<sethost>/<seturi> COSM/HTTP Put Control"},
    {"date",      	CMD_Date,   			" : <update>, <offset> <set> - Display/update date time via NTP, apply offset in minutes offset to the clock"},
    {"rgb",			CMD_Rgb,				" : <mode> <toprgb> <bottomrgb> - Mode 0-4, colours 32bit hex (html format)"},
    {"reboot",		CMD_Reboot,	 			" : Reboot"},
    {"uptime",		CMD_Uptime,	 			" : Power up time"},
    {"relay",		CMD_Relay,	 			" : <enabled>/<disabled> / optional:<relaynumber> <on/off>"},
    {"getadcs",		CMD_Adcs,	 			" : return ADC values for each port"},
    {"servomove",	CMD_ServoMove,			" : <servo> <position> - 8bit servo number (zero based) and position"},
    {"bridge",		CMD_BridgeScan,			" : <scan> List Connected SolderBridges"},
    {"dmxupdate",	CMD_NotImplemented,		" : offset value"},
    {"gettemp", 	CMD_GetTemperature,		" : Retrieve Temperature"},
    {"gpioinit",	CMD_GpioInit,	 		" : <disable>/<enable> / <get>/<set> <portno> <direction>/<port> <mask> <value> get or set port init"},
    {"gpio",		CMD_Gpio,	 			" : <get>/<set> <portno> <direction>/<port> <mask> <value> get port status or set dir/port value"},
    {"selftest",	CMD_SelfTest,			" : Factory Test with test rig"},
    //{"getlogic",	CMD_GetLogic,			" : optional:<number> - List Logic Statements"},
    { 0, 0, 0 }
};


static const char * const g_pcHex = "0123456789abcdef";
const int NUM_CMD = sizeof(g_sCmdTable)/sizeof(tCmdLineEntry);
bool SerialPrintBridgeScanResult = false;

#endif
