/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2012 - www.soldersplash.co.uk - C. Matthews - R. Steel

*/

#define _SERIAL_CONTROL_
#include "SplashBaseHeaders.h"
#include <String.h>


//*****************************************************************************
//
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
		Serial_PrintBridgeList();

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
			UARTprintf(" - %s", SystemConfig.splashBaseName);
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
// Serial_Init - Configure the IO for serial use
//
//*****************************************************************************
void Serial_Init ( void )
{
	if ( 0 == SERIAL_UART )
	{
		//
		// Configure the Port 0 pins appropriately.
		//
		GPIOPinTypeUART(PIN_U0RX_PORT, PIN_U0RX_PIN);
		GPIOPinTypeUART(PIN_U0TX_PORT, PIN_U0TX_PIN);

		UARTStdioInit(SERIAL_UART);

		// Stop the user controlling these pins
		UserGpio_AppSetMask(USER_GPIO_PORTA, PIN_U0RX_PIN | PIN_U0TX_PIN);
	}
	else if ( 1 == SERIAL_UART )
	{
		//
		// Configure the Port 1 pins appropriately.
		//
		GPIOPinTypeUART(PIN_U1RX_PORT, PIN_U1RX_PIN);
		GPIOPinTypeUART(PIN_U1TX_PORT, PIN_U1TX_PIN);

		UARTStdioInit(SERIAL_UART);

		// Stop the user controlling these pins
		UserGpio_AppSetMask(USER_GPIO_PORTD, PIN_U1RX_PIN | PIN_U1TX_PIN);
	}
}

//*****************************************************************************
//
// Print the select GPIO port info
//
//*****************************************************************************
void Serial_PrintGpioDetails ( ui8 portNo )
{
bool result = false;
ui32 portVal = 0;
ui32 portDir = 0;

	if ( UserGpioGet(portNo, &portVal) )
	{
		if ( UserGpioDirGet(portNo, &portDir) )
		{
			result = true;
			UARTprintf("%u : Port(%c) : Dir : 0x%04X Pins : 0x%04X \n", portNo, GPIO_PORT_LETTERS[portNo], portDir, portVal);
		}
	}

	if (! result )
	{
		UARTprintf("Error Invalid Port");
	}
}

//*****************************************************************************
//
// Serial_PrintBridgeList
//
// Lists Connected SolderBridges
//
//*****************************************************************************
void Serial_PrintBridgeList ( void )
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
}

//*****************************************************************************
//
// Command: setname
//
// Reboots the splashbase
//
//*****************************************************************************
int CMD_SetName (int argc, char **argv)
{
	if (argc > 1)
	{
		SysSetBaseName( (ui8 *)&argv[1][0], strlen(&argv[1][0]));
	}
	else
	{
		UARTprintf( "%s \n", (const char *)SystemConfig.splashBaseName );
	}

	return (0);
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
bool printConf = false;
volatile struct ip_addr ipaddr;
ui8 *tempChar;
ui8 macAddr[8];
ui32 tempIp = 0;

    (void) argc;
    (void) argv;

    tempChar = (ui8 *)&temp;

    if (argc > 1)
    {
    	if ( 'd' == argv[1][0] )
    	{
    		// set to dynamic ip
    		//SystemConfig.flags &= ~CONFIG_FLAG_STATICIP;
    		SystemConfig.flags.StaticIp = false;

    		// Reconfigure the ethernet
    		Ethernet_ReConfig();

    		// Save the change
    		SysConfigSave();

    		printConf = true;
    	}
    	else if ( 's' == argv[1][0] )
    	{
			if (argc > 4)
			{
				printConf = true;

				tempIp = inet_addr(&argv[2][0]);
				if ( INADDR_NONE != tempIp )
				{
					// Valid Ip
					SystemConfig.ulStaticIP = htonl(tempIp);
				}
				else
				{
					printConf = false;
				}

				tempIp = inet_addr(&argv[3][0]);
				if ( INADDR_NONE != tempIp )
				{
					// Valid Netmask Ip
					SystemConfig.ulSubnetMask = htonl(tempIp);
				}
				else
				{
					printConf = false;
				}

				tempIp = inet_addr(&argv[4][0]);
				if ( INADDR_NONE != tempIp )
				{
					// Valid Gateway Ip
					SystemConfig.ulGatewayIP = htonl(tempIp);
				}
				else
				{
					printConf = false;
				}

				if (printConf)
				{
					// Everything accepted
					//SystemConfig.flags |= CONFIG_FLAG_STATICIP;
					SystemConfig.flags.StaticIp = true;
					Ethernet_ReConfig();

					// Save the change
					SysConfigSave();
				}
				else
				{
					// Error revert to dynamic
					//SystemConfig.flags &= ~CONFIG_FLAG_STATICIP;
					SystemConfig.flags.StaticIp = false;
				}

			}
    	}
    }
    else
    {
    	// Just print
    	printConf = true;
    }

    if ( printConf )
    {
		//if ( SystemConfig.flags & CONFIG_FLAG_STATICIP )
    	if ( SystemConfig.flags.StaticIp )
		{
			UARTprintf("Static IP\n");
		}
		else
		{
			UARTprintf("Dynamic IP\n");
		}

		Ethernet_GetMacAddress(&macAddr[0]);
		UARTprintf("MAC : %02X-%02X-%02X-%02X-%02X-%02X \n", macAddr[0],macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5] );

		temp = Ethernet_GetIp();
		UARTprintf("IP : %d.%d.%d.%d \n", tempChar[3], tempChar[2], tempChar[1], tempChar[0]);

		temp = Ethernet_GetGatewayIp();
		UARTprintf("Gateway : %d.%d.%d.%d \n", tempChar[3], tempChar[2], tempChar[1], tempChar[0]);

		temp = Ethernet_GetNetmask();
		UARTprintf("Netmask : %d.%d.%d.%d \n", tempChar[3], tempChar[2], tempChar[1], tempChar[0]);
    }
    else
    {
    	UARTprintf("Error\n");
    }



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
//
//*****************************************************************************
int
CMD_Date (int argc, char **argv)
{
tTime currentTime;

    (void) argc;
    (void) argv;

    if (argc > 1)
    {
		if ( 'u' == argv[1][0] )
		{
			UARTprintf("Updating Clock Via SNTP \n");
			SntpGetTime();
		}
		else if ( 'o' == argv[1][0] )
		{
			// offset

			if ((argc > 2) && ( 's' == argv[2][0] ))
			{
				// set - update the time offset on the clock, value in signed minutes
				if ('-' == argv[3][0])
				{
					// The name lies, the TI ustrtoul will detect a negative value and give the right result
					SystemConfig.timeOffset = ustrtoul((const char *)&argv[3][0], 0, 10);
				}
				else
				{
					SystemConfig.timeOffset = ustrtoul((const char *)&argv[3], 0, 10);
				}
			}

			// Print current offset
			UARTprintf("Time offset : %i\n", SystemConfig.timeOffset );
		}
		// server
		else if ( 's' == argv[1][0] )
		{
			// if we get the set command followed by an address, update the SNTP server
			if (( argc > 3 ) && ( 's' == argv[2][0] ))
			{
				SysSetSntpAddress( (ui8 *)&argv[3][0], strlen(&argv[3][0]) );
			}

			// Print current server
			UARTprintf("NTP Server : %s\n", SystemConfig.sntpServerAddress );
		}
    }
	else
	{
		ulocaltime(Time_StampNow(SystemConfig.timeOffset), &currentTime);
		UARTprintf("%02d-%02d-%02d %02d:%02d:%02d - Offset : %d minutes\n", currentTime.usYear, (currentTime.ucMon+1), currentTime.ucMday, currentTime.ucHour, currentTime.ucMin, currentTime.ucSec, SystemConfig.timeOffset);
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
					RelayControl( command, 0x01);
				break;

				case '2' :
					RelayControl( command<<1, 0x02);
				break;

				case '3' :
					RelayControl( command<<2, 0x04);
				break;

				case '4' :
					RelayControl( command<<3, 0x08);
				break;
			}
		}
	}
	else if ( argc > 1 )
	{
		if ( 'n' == argv[1][1] )
		{
			// On
			RelayControl( 0x0F, 0x0F);
		}
		else if ( 'f' == argv[1][1] )
		{
			// off
			RelayControl( 0x00, 0x0F);
		}
		else if ( 'd' == argv[1][0] )
		{
			// disable
			SysConfigRelayDisable();
			UARTprintf("Relay SolderBridge control disabled\n");
		}
		else if ( 'e' == argv[1][0] )
		{
			// enable
			SysConfigRelayEnable();
			UARTprintf("Relay SolderBridge control disabled\n");
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

	if ( argc > 1 )
	{
		// scan
		if ( 's' == argv[1][0] )
		{
			// Kick off the scan. On SPI and I2C
			SolderBridge_StartScan();
			ExtGpio_Scan();
		}
	}

	// Next entry to the serial module will print out the bridge list
	SerialPrintBridgeScanResult = true;

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
	if ( argc > 1 )
	{
		// factory
		if ( 'f' == argv[1][0] )
		{
			SysConfigFactoryDefault();
			SysConfigSave();
			UARTprintf("Reverted To Factory Defaults\n");
		}
		else if ( 's' == argv[1][0] )
		{
			SysConfigSave();
			UARTprintf("System Config Saved\n");
		}
	}
	else
	{
		// TODO : Print config, how much?
	}

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

ui32 tempSensor = AdcGetTemperature();

	UARTprintf("MCU : %u°C - Sensor : %u°C\n", internalDegC, tempSensor);

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

	/*
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
	 */

	return (0);
}

//*****************************************************************************
//
// htoi - Ascii Hex to Integer
//
//
//*****************************************************************************
ui32 htoi ( char *s )
{
ui32 result = 0;
ui8 i = 0;
ui8 x = 0;

	for (i=0; s[i] !='\0'; i++ )
	{
		for (x=0; g_pcHex[x] != '\0'; x++ )
		{
			if ( s[i] == g_pcHex[x])
			{
				result <<= 4;
				result |= x;
				break;
			}
		}
	}

	return(result);
}

//*****************************************************************************
//
// Command: CMD_SetPortDir
//
//
//
//*****************************************************************************
int CMD_SetPortDir (int argc, char **argv)
{
ui32 portVal = 0;
ui32 mask = 0;
ui8 port = 0;

	if ( argc > 3 )
	{
		port = ustrtoul(argv[1], 0, 10);
		mask = htoi(argv[2]);
		portVal = htoi(argv[3]);

		UserGpioDirection( port, mask, portVal );
	}

	return (0);
}

//*****************************************************************************
//
// Command: CMD_GpioInit
//
//
//
//*****************************************************************************
int CMD_GpioInit (int argc, char **argv)
{
ui8 portNo = 0;
ui32 mask = 0;
ui32 newVal = 0;

	if ( argc > 1 )
	{
		if ( 'g' == argv[1][0] )
		{
			if ( argc > 2 )
			{
				// Only want one
				portNo = ustrtoul(&argv[2][0], 0, 10);
				if ( portNo < GPIO_PORT_TOTAL )
				{
					UARTprintf("%u : Port(%c) : Dir : 0x%04X Pins : 0x%04X \n", portNo, GPIO_PORT_LETTERS[portNo], SystemConfig.UserGpioInit[portNo][0], SystemConfig.UserGpioInit[portNo][1]);
				}
				else
				{
					UARTprintf("Error \n");
				}
			}
			else
			{
				for (portNo=0; portNo<GPIO_PORT_TOTAL; portNo++)
				{
					UARTprintf("%u : Port(%c) : Dir : 0x%04X Pins : 0x%04X \n", portNo, GPIO_PORT_LETTERS[portNo], SystemConfig.UserGpioInit[portNo][0], SystemConfig.UserGpioInit[portNo][1]);
				}
			}
		}
		else if ( 's' == argv[1][0] )
		{
			if ( argc > 5 )
			{
				portNo = ustrtoul(argv[2], 0, 10);
				mask = htoi(argv[4]);
				newVal = htoi(argv[5]);

				// "direction"
				if ( 'd' == argv[3][0] )
				{
					// We are changing a GPIO direction
					// TODO : Think some more about this ...
					newVal = mask & newVal;
					SystemConfig.UserGpioInit[portNo][0] = newVal;

					// Echo result
					UARTprintf("%u : Port(%c) : Dir : 0x%04X Pins : 0x%04X \n", portNo, GPIO_PORT_LETTERS[portNo], SystemConfig.UserGpioInit[portNo][0], SystemConfig.UserGpioInit[portNo][1]);

				}
				// "port"
				else if ( 'p' == argv[3][0] )
				{
					// Were changing a port output
					newVal = mask & newVal;
					SystemConfig.UserGpioInit[portNo][1] = newVal;

					// Echo result
					UARTprintf("%u : Port(%c) : Dir : 0x%04X Pins : 0x%04X \n", portNo, GPIO_PORT_LETTERS[portNo], SystemConfig.UserGpioInit[portNo][0], SystemConfig.UserGpioInit[portNo][1]);
				}
			}
		}
		else if ( 'd' == argv[1][0] )
		{
			// "disable" the GPIO init
			SystemConfig.flags.UserGpioInit = false;
			SysConfigSave();
			UARTprintf("User GPIO Initialisation Disabled\n");
		}
		else if ( 'e' == argv[1][0] )
		{
			// "enable" the GPIO init
			SystemConfig.flags.UserGpioInit = true;
			SysConfigSave();
			UARTprintf("User GPIO Initialisation Enabled\n");
		}
	}
	return (0);
}

//*****************************************************************************
//
// Command: CMD_Gpio
//
//
//
//*****************************************************************************
int CMD_Gpio (int argc, char **argv)
{
ui8 portNo = 0;
ui32 newVal = 0;
ui32 mask = 0;

	if ( argc > 1 )
	{
		if ( 'g' == argv[1][0] )
		{
			if ( argc > 2 )
			{
				// Only want one
				portNo = ustrtoul(&argv[2][0], 0, 10);
				Serial_PrintGpioDetails( portNo );
			}
			else
			{
				// List them all
				for (portNo=0; portNo<GPIO_PORT_TOTAL; portNo++)
				{
					Serial_PrintGpioDetails( portNo );
				}
			}
		}
		else if ( 's' == argv[1][0] )
		{
			if ( argc > 5 )
			{
				portNo = ustrtoul(argv[2], 0, 10);
				mask = htoi(argv[4]);
				newVal = htoi(argv[5]);

				// "direction"
				if ( 'd' == argv[3][0] )
				{
					// We are changing a GPIO direction
					UserGpioDirection( portNo, mask, newVal );

					// Read back the direction register and show the user
					// There maybe protected direction pins that we could not change so we dont assume we did!
					Serial_PrintGpioDetails( portNo );

				}
				// "port"
				else if ( 'p' == argv[3][0] )
				{
					// Were changing a port output
					UserGpioSetOutputs( portNo, mask, newVal );

					Serial_PrintGpioDetails( portNo );
				}
			}
		}
	}
	return (0);
}

//*****************************************************************************
//
// Command: CMD_SelfTest
//
//*****************************************************************************
int CMD_SelfTest (int argc, char **argv)
{
	SelfTest();

	return (0);
}

//*****************************************************************************
//
// Command: CMD_cosm
//
//*****************************************************************************
int CMD_cosm (int argc, char **argv)
{
ui16 stringLen = 0;

	if ( argc > 2 )
	{
		if (( 's' == argv[1][0] ) && ( 'k' == argv[1][3] ))
		{
			// "setkey"
			stringLen = strlen( &argv[2][0] );
			if ( stringLen < COSM_API_KEY_LEN-1 )
			{
				memcpy( &SystemConfig.cosmPrivKey[0], &argv[2][0], stringLen);
				SystemConfig.cosmPrivKey[stringLen]=0;
				SysConfigSave();
			}
		}
		else if (( 's' == argv[1][0] ) && ( 'h' == argv[1][3] ))
		{
			// "sethost"
			stringLen = strlen( &argv[2][0] );
			if ( stringLen < COSM_HOST_LEN-1 )
			{
				memcpy( &SystemConfig.cosmHost[0], &argv[2][0], stringLen);
				SystemConfig.cosmHost[stringLen] = 0;
				CosmHostUpdated();
				SysConfigSave();
			}
		}
		else if (( 's' == argv[1][0] ) && ( 'u' == argv[1][3] ))
		{
			// "seturi"
			stringLen = strlen( &argv[2][0] );
			if ( stringLen < COSM_URL_LEN-1 )
			{
				memcpy( &SystemConfig.cosmUrl[0], &argv[2][0], stringLen);
				SystemConfig.cosmUrl[stringLen] = 0;
				SysConfigSave();
			}
		}

		UARTprintf("Host : %s - URI : %s - Private Key : %s \n", SystemConfig.cosmHost, SystemConfig.cosmUrl, SystemConfig.cosmPrivKey);
	}
	else if ( argc > 1 )
	{
		if ( 'u' == argv[1][0] )
		{
			// "update"
			CosmGetIp();
		}

		UARTprintf("COSM Update Triggered \n");
	}
	else
	{
		UARTprintf("Host : %s - URI : %s - Private Key : %s \n", SystemConfig.cosmHost, SystemConfig.cosmUrl, SystemConfig.cosmPrivKey);
	}

	return (0);
}

