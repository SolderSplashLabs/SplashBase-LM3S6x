/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2012 - www.soldersplash.co.uk - C. Matthews - R. Steel

*/

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"

#include "utils/ustdlib.h"
#include "utils/uartstdio.h"
#include "utils/cmdline.h"

#include "globals.h"
#include "datatypes.h"

#include "time.h"
#include "config.h"
#include "globals.h"
#include "logicController.h"

#include "solderBridge\externalGpio.h"

// TODO : CPU Temp

//*****************************************************************************
//
// Input buffer for the command line interpreter.
//
//*****************************************************************************
static char g_cInput[200];

// Max no of arguments per command
#define CMDLINE_MAX_ARGS 5

//*****************************************************************************
//
// Declaration for the callback functions that will implement the command line
// functionality.  These functions get called by the command line interpreter
// when the corresponding command is typed into the command line.
//
//*****************************************************************************
extern int CMD_help (int argc, char **argv);
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

// Serial Banner, this looks mangled but in putty it looks good!
const char WELCOME_MSG[] = "\n  ____        _     _           ____        _           _	 _          _\n\
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___ \n\
 \\___ \\ / _ \\| |/ _` |/ _ \\ '__\\___ \\| '_ \\| |/ _` / __| '_ \\	| |   / _` | '_ \\/ __| \n\
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \\__ \\ | | |	| |__| (_| | |_) \\__ \\ \n\
 |____/ \\___/|_|\\__,_|\\___|_|  |____/| .__/|_|\\__,_|___/_| |_|	|_____\\__,_|_.__/|___/ \n\
                                     |_| \nSolderSplash Labs - SplashBase V";

const char ACK_STRING[] = "Acknowledged\n";

//*****************************************************************************
//
// Table of valid command strings, callback functions and help messages.
//
//*****************************************************************************
tCmdLineEntry g_sCmdTable[] =
{
    {"help",     	CMD_help,      			" : Display list of commands" },
    {"?",     		CMD_help,      			" : Display list of commands" },
    {"setname",  	CMD_NotImplemented,     " : name - Set SplashBase name"},
    {"ipconfig", 	CMD_ipconfig,  			" : Show network config"},
    {"date",      	CMD_Date,   			" : <update> - Display the date and time. optional update command to force an update via NTP"},
    {"rgb",			CMD_Rgb,				" : <mode> <toprgb> <bottomrgb> - Mode 0-4, colours 32bit hex (html format)"},
    {"reboot",		CMD_Reboot,	 			" : Reboot"},
    {"uptime",		CMD_Uptime,	 			" : Power up time"},
    {"relay",		CMD_Relay,	 			" : optional:<relaynumber> <on/off>"},
    {"setport",		CMD_NotImplemented,	 	" : <portletter> <mask> <portdata> - set port output"},
    {"getport",		CMD_NotImplemented,	 	" : <portletter> - get port status"},
    {"setportdir",	CMD_NotImplemented,	 	" : <portletter> mask direction - set port direction"},
    {"getportdir",	CMD_NotImplemented,	 	" : <portletter> - get port direction"},
    {"getadcs",		CMD_Adcs,	 			" : return ADC values for each port"},
    {"factorydefault",		CMD_Factory,	" : factory default settings"},
    {"servomove",	CMD_ServoMove,			" : <servo> <position> - 8bit servo number (zero based) and position"},
    {"bridgescan",	CMD_BridgeScan,			" : Scan for SPI bridges"},
    {"bridgelist",	CMD_BridgeList,			" : List SPI bridges"},
    {"dmxupdate",	CMD_NotImplemented,		" : offset value"},
    {"gettemp", 	CMD_GetTemperature,		" : Retrieve Temperature"},
    {"getlogic",	CMD_GetLogic,			" : optional:<number> - List Logic Statements"},
    { 0, 0, 0 }
};


const int NUM_CMD = sizeof(g_sCmdTable)/sizeof(tCmdLineEntry);

bool SerialPrintBridgeScanResult = false;

//*****************************************************************************
// Serial_Task
//
//*****************************************************************************
void Serial_Task ( void )
{
long lCommandStatus;
static char first = true;

	if (SerialPrintBridgeScanResult)
	{
		// Print the bridgelist
		CMD_BridgeList(0,0);

		SerialPrintBridgeScanResult = false;

		UARTprintf(">");
	}

	if (UARTPeek('\r') != -1)
	{
		if ( first )
		{
			UARTprintf(WELCOME_MSG);
			UARTprintf("%d.%d - ",SW_REV_MAJOR, SW_REV_MINOR);
			UARTprintf(__DATE__);
			UARTprintf(" ");
			UARTprintf(__TIME__);
			UARTprintf("\n");

			first = false;
		}

		//
		// a '\r' was detected get the line of text from the user.
		//
		UARTgets(g_cInput,sizeof(g_cInput));

		//
		// Pass the line from the user to the command processor.
		// It will be parsed and valid commands executed.
		//
		lCommandStatus = CmdLineProcess(g_cInput);

		//
		// Handle the case of bad command.
		//
		if(lCommandStatus == CMDLINE_BAD_CMD)
		{
			//UARTprintf("Bad command!\n");
		}

		//
		// Handle the case of too many arguments.
		//
		else if(lCommandStatus == CMDLINE_TOO_MANY_ARGS)
		{
			UARTprintf("Too many arguments for command processor!\n");
		}

		if (SerialPrintBridgeScanResult)
		{

		}
		else
		{
			UARTprintf(">");
		}
	}

}



//*****************************************************************************
//
// Command: help
//
// Print the help strings for all commands.
//
//*****************************************************************************
int
CMD_help (int argc, char **argv)
{
    int index;

    (void)argc;
    (void)argv;

    UARTprintf("\n");
    for (index = 0; index < NUM_CMD-1; index++)
    {
      UARTprintf("%17s %s\n",
        g_sCmdTable[index].pcCmd,
        g_sCmdTable[index].pcHelp);
    }
    UARTprintf("\n");

    return (0);
}


//*****************************************************************************
//
// Command: Not Implemented
//
//
//*****************************************************************************
int CMD_NotImplemented (int argc, char **argv)
{
(void) argc;
(void) argv;

    UARTprintf("Command not availible \n");

    return (0);
}


//*****************************************************************************
//
// Command: ipconfig
//
// Show the network configuration/status
//
//*****************************************************************************
int CMD_ipconfig (int argc, char **argv)
{
ui32 temp;
ui8 *tempChar;
ui8 macAddr[8];

    (void) argc;
    (void) argv;

    tempChar = (ui8 *)&temp;

    if ( g_sParameters.ucFlags & CONFIG_FLAG_STATICIP )
    {
    	UARTprintf("Static IP\n");
    }
    else
    {
    	UARTprintf("Dynamic IP\n");
    }

    Ethernet_GetMacAddress(&macAddr);
    UARTprintf("MAC : %02X-%02X-%02X-%02X-%02X-%02X \n", macAddr[0],macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5] );

    temp = Ethernet_GetIp();
    UARTprintf("IP : %d.%d.%d.%d \n", tempChar[0], tempChar[1], tempChar[2], tempChar[3]);

    temp = Ethernet_GetGatewayIp();
    UARTprintf("Gateway : %d.%d.%d.%d \n", tempChar[0], tempChar[1], tempChar[2], tempChar[3]);

    temp = Ethernet_GetNetmask();
    UARTprintf("Netmask : %d.%d.%d.%d \n", tempChar[0], tempChar[1], tempChar[2], tempChar[3]);

    return (0);
}

//*****************************************************************************
//
// Command: reboot
//
// Reboots the splashbase
//
//*****************************************************************************
int
CMD_Reboot (int argc, char **argv)
{
    (void) argc;
    (void) argv;

    UARTprintf("Rebooting \n");

    HWREG(NVIC_APINT) = (NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ);

    return (0);
}

//*****************************************************************************
//
// Command: date
//
// gets or updates the date/time
// TODO : Currently assumes GMT
//
//*****************************************************************************
int
CMD_Date (int argc, char **argv)
{
tTime currentTime;

    (void) argc;
    (void) argv;

    //if ((argc > 1) && ( *(argv[1]) == 'u'))
    if ((argc > 1) && ( argv[1][0] == 'u'))
    {
    	UARTprintf("Updating Clock Via SNTP \n");
    	SntpGetTime();
    }
    else
    {
    	ulocaltime(Time_StampNow(), &currentTime);
    	UARTprintf("%02d-%02d-%02d %02d:%02d:%02d GMT\n", currentTime.usYear, (currentTime.ucMon+1), currentTime.ucMday, currentTime.ucHour, currentTime.ucMin, currentTime.ucSec);
    }

    return (0);
}


int CMD_Relay (int argc, char **argv)
{
ui8 command = 2;
	(void) argc;
	(void) argv;

	if ( argc > 2 )
	{
		if ( 'n' == argv[2][1] )
		{
			command = 1;
		}
		else if ( 'f' == argv[2][1] )
		{
			command = 0;
		}

		if (command < 2)
		{
			switch ( argv[1][0] )
			{
				case '1' :
					relayControl( command, 0x01);
				break;

				case '2' :
					relayControl( command<<1, 0x02);
				break;

				case '3' :
					relayControl( command<<2, 0x04);
				break;

				case '4' :
					relayControl( command<<3, 0x08);
				break;
			}
		}
	}
	else if ( argc > 1 )
	{
		if ( 'n' == argv[1][1] )
		{
			relayControl( 0x0F, 0x0F);
		}
		else if ( 'f' == argv[1][1] )
		{
			relayControl( 0x00, 0x0F);
		}
	}

	return (0);
}


//*****************************************************************************
//
// Command: bridgeScan
//
// Starts an SPI bridge scan
//
//*****************************************************************************
int CMD_BridgeScan (int argc, char **argv)
{
	(void) argc;
	(void) argv;

	/*
	if ( SolderBridge_StartScan() )
	{
		UARTprintf("Scanning SPI for Bridges\n");
	}
	else
	{
		UARTprintf("Busy\n");
	}
	*/

	SolderBridge_StartScan();
	ExtGpio_Scan();

	SerialPrintBridgeScanResult = true;

	return (0);
}


//*****************************************************************************
//
// Command: bridgelist
//
// Lists SPI Bridges on the SPI bus
//
//*****************************************************************************
int CMD_BridgeList (int argc, char **argv)
{
ui8 i = 0;

	for ( i=0; i<5; i++ )
	{
		UARTprintf("CS%u : %s \n", i, SB_GetBridgeName(i));
	}

	for ( i=0; i<8; i++ )
	{
		if ( IoExpanders.pcaAvailible & (0x01<<i) )
		{
			UARTprintf("I/O Expander, i2c Address : 0x%02X \n", (0x20+i));
		}
	}

	return (0);
}

//*****************************************************************************
//
// Command: rand
//
// Starts the automatic light sequence immediately.
//
//*****************************************************************************
int
CMD_Adcs (int argc, char **argv)
{
	ui16 adcValues[3];

	AdcAllAdcResults(&adcValues[0], sizeof(adcValues));
	UARTprintf("%u, %u, %u\n", adcValues[0], adcValues[1], adcValues[2]);

	return (0);
}


//*****************************************************************************
//
// Command: factory
//
// Starts the automatic light sequence immediately.
//
//*****************************************************************************
int CMD_Factory (int argc, char **argv)
{
	ConfigLoadFactory();
	ConfigSave();
	ConfigUpdateAllParameters(true);
	UARTprintf("Reverted To Factory Defaults\n");

	return (0);
}


//*****************************************************************************
//
// Command: uptime
//
// Starts the automatic light sequence immediately.
//
//*****************************************************************************
int CMD_Uptime (int argc, char **argv)
{
ui32 uptime = 0;
ui16 days = 0;
ui8 hours = 0;
ui8 minutes = 0;

	uptime = Time_Uptime();

	days = uptime / SECONDS_IN_AN_DAY;
	uptime -= days * SECONDS_IN_AN_DAY;

	hours = uptime / SECONDS_IN_AN_HOUR;
	uptime -= hours * SECONDS_IN_AN_HOUR;

	minutes = uptime / SECONDS_IN_AN_MIN;
	uptime -= minutes * SECONDS_IN_AN_MIN;

	UARTprintf("%d day/s %02d:%02d:%02d \n", days, hours, minutes, uptime);

	return (0);
}


//*****************************************************************************
//
// Command: servomove
//
// command a servo to move to a position
//
//*****************************************************************************
int CMD_ServoMove (int argc, char **argv)
{
ui8 servo = 0;
ui8 position = 0;

	(void) argc;
	(void) argv;

	servo = ustrtoul(argv[1], 0, 10);
	position = ustrtoul(argv[2], 0, 10);

	if ( ServoMoveToPos(servo, position) )
	{
		UARTprintf("Servo Moved\n");
	}
	else
	{
		UARTprintf("No Servo\n");
	}

	return (0);
}

//*****************************************************************************
//
// Command: rgb
//
// update RGB demand
//
//*****************************************************************************
int CMD_Rgb (int argc, char **argv)
{
ui8 mode = 0;
ui32 top = 0;
ui32 bottom = 0;

	mode = ustrtoul(argv[1], 0, 10);
	top = ustrtoul(argv[2], 0, 16);
	bottom = ustrtoul(argv[3], 0, 16);

	// TODO : need to set no of ticks and steps
	ColourModeSet( mode, top, bottom, 255, 1 );
	UARTprintf(ACK_STRING);

	return (0);
}

//*****************************************************************************
//
// Command: rand
//
// Starts the automatic light sequence immediately.
//
//*****************************************************************************
int
CMD_rand (int argc, char **argv)
{
(void) argc;
(void) argv;
    //g_sAppState.ulMode = APP_MODE_AUTO;

    return (0);
}

//*****************************************************************************
//
// Command: CMD_GetTemperature
//
// Display Temperature
//
//*****************************************************************************
int CMD_GetTemperature (int argc, char **argv)
{
ui32 internalDegC = AdcGetInternalTemp();

	UARTprintf("MCU : %u°C\n", internalDegC);

	return (0);
}

//*****************************************************************************
//
// Command: CMD_GetLogic
//
// Show logic commands
//
//*****************************************************************************
int CMD_GetLogic (int argc, char **argv)
{
ui8 i = 0;

	UARTprintf("\n");

	for ( i=0; i<10; i++ )
	{
		if ( LogicConditions[i].active )
		{
			UARTprintf("%u : IF ( %s ) ", i, LogicGetEventStr(i, true));

			if ( LogicConditions[i].andEventParam2 )
			{
				UARTprintf("AND ( %s ) ", LogicGetEventStr(i, false));
			}

			UARTprintf("THEN ( %s ) \n", LogicGetActionStr(i));
		}
	}

	return (0);
}

