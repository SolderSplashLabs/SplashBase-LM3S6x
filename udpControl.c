/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2012 - www.soldersplash.co.uk - C. Matthews - R. Steel

*/

#define UDPCONTROL
#include "SplashBaseHeaders.h"

#include <string.h>

//*****************************************************************************
//
// SSC_Init - Start listening on a socket for commands
//
//*****************************************************************************
void SSC_Init(void)
{
    //
    // Create a new UDP port for listening for commands.
    //
    UdpControlPort = udp_new();
    udp_recv(UdpControlPort, SSC_Recieve, NULL);
    udp_bind(UdpControlPort, IP_ADDR_ANY, SSC_UDP_PORT_RX);
    //udp_connect(pcb, IP_ADDR_ANY, SSC_UDP_PORT_RX);
}

//*****************************************************************************
//
// SSC_MACAddrSet - sets the MAC address advertised via UDP
//
//*****************************************************************************
void SSC_MACAddrSet(unsigned char *pucMACArray)
{
    //
    // Save the MAC address into our message buffer
    //
    SscReplyBuffer[5] = pucMACArray[0];
    SscReplyBuffer[6] = pucMACArray[1];
    SscReplyBuffer[7] = pucMACArray[2];
    SscReplyBuffer[8] = pucMACArray[3];
    SscReplyBuffer[9] = pucMACArray[4];
    SscReplyBuffer[10] = pucMACArray[5];
}

//*****************************************************************************
//
// SSC_SetUnitName
// Sets the unit name that is used when talking to the network
//
//*****************************************************************************
void SSC_SetUnitName(const ui8 *pcAppTitle)
{
ui8 strLenCnt = 0;

	// Todo : Magic numbers ...

	// Loop until you hit the max string length or find a 0 in the string
	for(strLenCnt = 0; (strLenCnt < SPLASHBASE_NAME_LEN-1) && *pcAppTitle; strLenCnt++)
	{
		SscReplyBuffer[strLenCnt + SSC_POS_UNITNAME] = *pcAppTitle++;
	}

	//
	// Zero-fill the remainder of the space in the response data (if any).
	//
	for(; strLenCnt < SPLASHBASE_NAME_LEN-1; strLenCnt++)
	{
		SscReplyBuffer[strLenCnt + SSC_POS_UNITNAME] = 0;
	}
}

//*****************************************************************************
// SSC_SetUnitName
// Sets the unit name that is used when talking to the network
//
//*****************************************************************************
void SSC_SetRelayNames ( void )
{
	memcpy( &SscReplyBuffer[SSC_POS_RELAY1NAME], (char *)&SystemConfig.relayOneName, (SPLASHBASE_RELAYNAME_LEN * 4) );
}

//*****************************************************************************
//
// SSC_SendReply - Sends the SplashBase Status out
//
//*****************************************************************************
static void SSC_SendReply(struct ip_addr *addr)
{
struct pbuf *p;
ui8 i = 0;
unsigned long ulIPAddress;
ui16 temp;
volatile ui8 *pucData;
	
    //
    // Allocate a new pbuf for sending the response.
    //
    p = pbuf_alloc(PBUF_TRANSPORT, SSC_REPLY_LEN, PBUF_RAM);
    if(p == NULL)
    {
        return;
    }
    
    // Fill the array with data ..
    pucData = p->payload;
    
    // Message Header
    SscReplyBuffer[SSC_POS_STARTBYTE] = 0xE1;
    
    // Our IP
    ulIPAddress = lwIPLocalIPAddrGet();
    SscReplyBuffer[SSC_POS_IP] = 0x000000FF & (ulIPAddress >> 24);
    SscReplyBuffer[SSC_POS_IP + 1] = 0x000000FF & (ulIPAddress >> 16);
    SscReplyBuffer[SSC_POS_IP + 2] = 0x000000FF & (ulIPAddress >> 8);
    SscReplyBuffer[SSC_POS_IP + 3] = 0x000000FF & (ulIPAddress);
    
    // MAC address already in the buffer
    
   	// SW Rev - TODO : Move to Defines
   	SscReplyBuffer[SSC_POS_SWREV] = SW_REV_MAJOR;
   	SscReplyBuffer[SSC_POS_SWREV+1] = SW_REV_MINOR;
   	

   	SscReplyBuffer[SSC_POS_CONFIG_BITS] = (*(ui8 *)&SystemConfig.flags);

   	// ADCs
   	AdcAllAdcResults((ui16 *)&SscReplyBuffer[SSC_POS_ADC0], 3);

   	// Current Relay State
   	SscReplyBuffer[SSC_POS_RELAYSTATE] = RelayGetClosed();
    
    temp = pwmGetDuty(0);
    SscReplyBuffer[SSC_POS_PWM0DUTY] = temp;
    SscReplyBuffer[SSC_POS_PWM0DUTY + 1] = temp >> 8;
    
    temp = pwmGetDuty(1);
    SscReplyBuffer[SSC_POS_PWM1DUTY] = temp;
    SscReplyBuffer[SSC_POS_PWM1DUTY + 1] = temp >> 8;
    
    temp = pwmGetDuty(2);
    SscReplyBuffer[SSC_POS_PWM2DUTY] = temp;
    SscReplyBuffer[SSC_POS_PWM2DUTY + 1] = temp >> 8;
    
    temp = pwmGetFreq();
    SscReplyBuffer[SSC_POS_PWMFREQ] = temp;
    SscReplyBuffer[SSC_POS_PWMFREQ + 1] = temp >> 8;
    
    // Get RGB Value
    SscReplyBuffer[SSC_POS_COLOURMODE] = ColourSettings.colourMode;
    SscReplyBuffer[SSC_POS_REDDUTY] = ColourSettings.bottomRed;
    SscReplyBuffer[SSC_POS_GREENDUTY] = ColourSettings.bottomGreen;
    SscReplyBuffer[SSC_POS_BLUEDUTY] = ColourSettings.bottomBlue;
    SscReplyBuffer[SSC_POS_STEPSIZE] = ColourSettings.ticksPerStep;
    
    SscReplyBuffer[SSC_POS_STEPCNT] = 0x00FF & ColourSettings.maxSteps;
    SscReplyBuffer[SSC_POS_STEPCNT + 1] = 0x00FF & (ColourSettings.maxSteps >> 8);
    
	// Unit name already in the buffer
	// As our relay names


    (*(ui32 *)&SscReplyBuffer[SSC_POS_TIMESTAMP]) = Time_StampNow(SystemConfig.timeOffset);

    //
    // Copy the response packet data into the pbuf.
    //
    pucData = p->payload;
    for(i = 0; i < sizeof(SscReplyBuffer); i++)
    {
        pucData[i] = SscReplyBuffer[i];
    }
    
    //
    // Send the response.
    //
    udp_sendto(UdpControlPort, p, addr, SSC_UDP_PORT_TX);

    //
    // Free the pbuf.
    //
    pbuf_free(p);	
}

//*****************************************************************************
//
// SSC_SendAllPortInfo - Sends the GPIO info message
//
//*****************************************************************************
void SSC_SendAllPortInfo ( struct ip_addr *addr )
{
volatile ui8 *pucData;
struct pbuf *p;
ui8 pos;
ui8 i = 0;
ui32 ulIPAddress = 0;
ui32 tempLong = 0;

	p = pbuf_alloc(PBUF_TRANSPORT, SSC_GPIO_ALL_REPLY, PBUF_RAM);

	if(p == NULL)
	{
	   // No ram!
	}
	else
	{
		pucData = p->payload;

		// Message Header
		pucData[SSC_POS_STARTBYTE] = SSC_REPLY_ALL_GPIO;

		// Our IP
		ulIPAddress = lwIPLocalIPAddrGet();
		pucData[SSC_POS_IP] = 0x000000FF & (ulIPAddress >> 24);
		pucData[SSC_POS_IP + 1] = 0x000000FF & (ulIPAddress >> 16);
		pucData[SSC_POS_IP + 2] = 0x000000FF & (ulIPAddress >> 8);
		pucData[SSC_POS_IP + 3] = 0x000000FF & (ulIPAddress);

		// MAC address already in the buffer
		Ethernet_GetMacAddress( (ui8 *)&pucData[SSC_POS_MAC] );

		// SW Rev - TODO : Move to Defines
		pucData[SSC_POS_SWREV] = SW_REV_MAJOR;
		pucData[SSC_POS_SWREV+1] = SW_REV_MINOR;

		pucData[SSC_POS_CONFIG_BITS] = (*(ui8 *)&SystemConfig.flags);
		pucData[SSC_POS_SPARE] = 0;

		// Total number of GPIO ports
		pucData[13] = GPIO_PORT_TOTAL;

		pos = 14;
		// Each GPIO port to follow
		for ( i=0; i<GPIO_PORT_TOTAL; i++ )
		{
			UserGpioDirGet(i, &tempLong);

			// We have no ports over 16bits ( mcu is 8bit io expanders are 16bit )
			(*(ui16 *)&pucData[pos]) = (ui16)tempLong;
			pos += 2;

			UserGpioGet(i, &tempLong);
			(*(ui16 *)&pucData[pos]) = (ui16)tempLong;
			pos += 2;

			(*(ui16 *)&pucData[pos]) = SystemConfig.UserGpioInit[i][0];
			pos += 2;

			(*(ui16 *)&pucData[pos]) = SystemConfig.UserGpioInit[i][1];
			pos += 2;

			(*(ui16 *)&pucData[pos]) = (ui16)UserGpio_AppMaskedIO(i);
			pos += 2;
		}

		if ( 0xFFFFFFFF == addr->addr )
		{
			// Broadcast
			addr = IP_ADDR_BROADCAST;
		}
		else
		{
			// Unicast
		}

		udp_sendto(UdpControlPort, p, addr, SSC_UDP_PORT_TX);

		//
		// Free the pbuf.
		//
		pbuf_free(p);
	}
}


//*****************************************************************************
//
// SSC_SendPortInfo - Sends the GPIO info message
//
//*****************************************************************************
void SSC_SendPortInfo ( struct ip_addr *addr, ui16 reason )
{
volatile ui8 *pucData;
struct pbuf *p;
ui8 pos;
ui8 i = 0;
ui32 ulIPAddress = 0;

	p = pbuf_alloc(PBUF_TRANSPORT, SSC_INFO_MSG_LEN, PBUF_RAM);

	if(p == NULL)
	{
	   // No ram!
	}
	else
	{
		pucData = p->payload;

		// Message Header
		pucData[SSC_POS_STARTBYTE] = 0xE1;

		// Our IP
		ulIPAddress = lwIPLocalIPAddrGet();
		pucData[SSC_POS_IP] = 0x000000FF & (ulIPAddress >> 24);
		pucData[SSC_POS_IP + 1] = 0x000000FF & (ulIPAddress >> 16);
		pucData[SSC_POS_IP + 2] = 0x000000FF & (ulIPAddress >> 8);
		pucData[SSC_POS_IP + 3] = 0x000000FF & (ulIPAddress);

		// MAC Addr
		Ethernet_GetMacAddress((ui8 *)&pucData[SSC_POS_MAC]);

		// SW Rev - TODO : Move to Defines & centralise
		pucData[SSC_POS_SWREV] = SW_REV_MAJOR;
		pucData[SSC_POS_SWREV+1] = SW_REV_MINOR;

		pos = SSC_POS_SWREV+2;
		pucData[pos++] = reason;
		pucData[pos++] = reason >> 8;

		// Total number of GPIO ports
		pucData[pos++] = GPIO_PORT_TOTAL;

		// Each GPIO port to follow
		for ( i=0; i<GPIO_PORT_TOTAL; i++ )
		{
			*(ui32 *)(&pucData[pos]) = 0;
			UserGpioGet(i, (ui32 *)&pucData[pos]);
			pos += 4;
		}

		if ( 0xFFFFFFFF == addr->addr )
		{
			// Broadcast
			addr = IP_ADDR_BROADCAST;
		}
		else
		{
			// Unicast
		}

		udp_sendto(UdpControlPort, p, addr, SSC_UDP_PORT_TX);

		//
		// Free the pbuf.
		//
		pbuf_free(p);
	}
}

//*****************************************************************************
//
// SSC_UpdatePwmDuty - Processes a PWM Duty message
//
//*****************************************************************************
void SSC_SendExtendedInfo( struct ip_addr *addr )
{
volatile ui8 *pucData;
struct pbuf *p;
ui8 i = 0;
ui8 x = 0;
ui32 ulIPAddress = 0;

	p = pbuf_alloc(PBUF_TRANSPORT, SSC_EXTD_REPLY_LEN, PBUF_RAM);

	if(p == NULL)
	{
	   // No ram!
		// TODO : Log
	}
	else
	{
		pucData = p->payload;

		// Message Header
		pucData[SSC_POS_STARTBYTE] = 0xE2;

		// Our IP
		ulIPAddress = lwIPLocalIPAddrGet();
		pucData[SSC_POS_IP] = 0x000000FF & (ulIPAddress >> 24);
		pucData[SSC_POS_IP + 1] = 0x000000FF & (ulIPAddress >> 16);
		pucData[SSC_POS_IP + 2] = 0x000000FF & (ulIPAddress >> 8);
		pucData[SSC_POS_IP + 3] = 0x000000FF & (ulIPAddress);

		// MAC Addr
		Ethernet_GetMacAddress((ui8 *)&pucData[SSC_POS_MAC]);

		pucData[11] = (*(ui8 *)&SystemConfig.flags);
		pucData[12] = 0;

		for ( i=0; i<5; i++ )
		{
			pucData[13 + i] = SB_GetBridgeType(i);
		}

		// Use x as the i2c bridge count
		x = 0;
		for ( i=0; i<8; i++ )
		{
			// Initalise as 0 first
			pucData[18 + i] = 0;

			if ( IoExpanders.pcaAvailible & (0x01<<i) )
			{
				pucData[18 + x] = (0x20+i);
				x++;
			}
		}

		ulIPAddress = lwIPLocalNetMaskGet();
		pucData[26] = 0x000000FF & (ulIPAddress >> 24);
		pucData[26 + 1] = 0x000000FF & (ulIPAddress >> 16);
		pucData[26 + 2] = 0x000000FF & (ulIPAddress >> 8);
		pucData[26 + 3] = 0x000000FF & (ulIPAddress);

		ulIPAddress = lwIPLocalGWAddrGet();
		pucData[30] = 0x000000FF & (ulIPAddress >> 24);
		pucData[30 + 1] = 0x000000FF & (ulIPAddress >> 16);
		pucData[30 + 2] = 0x000000FF & (ulIPAddress >> 8);
		pucData[30 + 3] = 0x000000FF & (ulIPAddress);

		memcpy( (ui8 *)&pucData[34], &SystemConfig.sntpServerAddress[0], SNTP_SERVER_LEN);

		// Timestamp
		pucData[66] = (ui8)(0x00FF & SystemConfig.timeOffset);
		pucData[67] = (ui8)(0x00FF & (SystemConfig.timeOffset >> 8));

		if ( 0xFFFFFFFF == addr->addr )
		{
			// Broadcast
			addr = IP_ADDR_BROADCAST;
		}
		else
		{
			// Unicast
		}

		udp_sendto(UdpControlPort, p, addr, SSC_UDP_PORT_TX);

		//
		// Free the pbuf.
		//
		pbuf_free(p);
	}
}

//*****************************************************************************
//
// SSC_SendLogicActions -
//
//*****************************************************************************
void SSC_SendLogicActions( struct ip_addr *addr )
{
volatile ui8 *pucData;
struct pbuf *p;
ui32 ulIPAddress = 0;

	p = pbuf_alloc(PBUF_TRANSPORT, SSC_REPLY_LEN_ACTION, PBUF_RAM);

	if(p == NULL)
	{
		// No ram!
		// TODO : Log
	}
	else
	{
		pucData = p->payload;

		// Message Header
		pucData[SSC_POS_STARTBYTE] = SSC_REPLY_LOGIC_ACT;

		// Our IP
		ulIPAddress = lwIPLocalIPAddrGet();
		pucData[SSC_POS_IP] = 0x000000FF & (ulIPAddress >> 24);
		pucData[SSC_POS_IP + 1] = 0x000000FF & (ulIPAddress >> 16);
		pucData[SSC_POS_IP + 2] = 0x000000FF & (ulIPAddress >> 8);
		pucData[SSC_POS_IP + 3] = 0x000000FF & (ulIPAddress);

		// MAC Addr
		Ethernet_GetMacAddress((ui8 *)&pucData[SSC_POS_MAC]);

		pucData[11] = (*(ui8 *)&SystemConfig.flags);
		pucData[12] = 0;

		pucData[13] = 24;

		LogicCopyActions( (ui8 *)&pucData[14] );

		if ( 0xFFFFFFFF == addr->addr )
		{
			// Broadcast
			addr = IP_ADDR_BROADCAST;
		}
		else
		{
			// Unicast
		}

		udp_sendto(UdpControlPort, p, addr, SSC_UDP_PORT_TX);

		//
		// Free the pbuf.
		//
		pbuf_free(p);
	}
}

//*****************************************************************************
//
// SSC_SendLogicConditions -
//
//*****************************************************************************
void SSC_SendLogicConditions( struct ip_addr *addr )
{
volatile ui8 *pucData;
struct pbuf *p;

ui32 ulIPAddress = 0;

	p = pbuf_alloc(PBUF_TRANSPORT, SSC_REPLY_LEN_COND, PBUF_RAM);

	if(p == NULL)
	{
	   // No ram!
		// TODO : Log
	}
	else
	{
		pucData = p->payload;

		// Message Header
		pucData[SSC_POS_STARTBYTE] = SSC_REPLY_LOGIC_COND;

		// Our IP
		ulIPAddress = lwIPLocalIPAddrGet();
		pucData[SSC_POS_IP] = 0x000000FF & (ulIPAddress >> 24);
		pucData[SSC_POS_IP + 1] = 0x000000FF & (ulIPAddress >> 16);
		pucData[SSC_POS_IP + 2] = 0x000000FF & (ulIPAddress >> 8);
		pucData[SSC_POS_IP + 3] = 0x000000FF & (ulIPAddress);

		// MAC Addr
		Ethernet_GetMacAddress((ui8 *)&pucData[SSC_POS_MAC]);

		pucData[11] = (*(ui8 *)&SystemConfig.flags);
		pucData[12] = 0;

		pucData[13] = 24;

		LogicCopyConditions( (ui8 *)&pucData[14] );

		if ( 0xFFFFFFFF == addr->addr )
		{
			// Broadcast
			addr = IP_ADDR_BROADCAST;
		}
		else
		{
			// Unicast
		}

		udp_sendto(UdpControlPort, p, addr, SSC_UDP_PORT_TX);

		//
		// Free the pbuf.
		//
		pbuf_free(p);
	}
}

//*****************************************************************************
//
// SSC_SendLogicEvents -
//
//*****************************************************************************
void SSC_SendLogicEvents( struct ip_addr *addr )
{
volatile ui8 *pucData;
struct pbuf *p;
ui32 ulIPAddress = 0;

	p = pbuf_alloc(PBUF_TRANSPORT, SSC_REPLY_LEN_EVENT, PBUF_RAM);

	if(p == NULL)
	{
	   // No ram!
		// TODO : Log
	}
	else
	{
		pucData = p->payload;

		// Message Header
		pucData[SSC_POS_STARTBYTE] = SSC_REPLY_LOGIC_EVENTS;

		// Our IP
		ulIPAddress = lwIPLocalIPAddrGet();
		pucData[SSC_POS_IP] = 0x000000FF & (ulIPAddress >> 24);
		pucData[SSC_POS_IP + 1] = 0x000000FF & (ulIPAddress >> 16);
		pucData[SSC_POS_IP + 2] = 0x000000FF & (ulIPAddress >> 8);
		pucData[SSC_POS_IP + 3] = 0x000000FF & (ulIPAddress);

		// MAC Addr
		Ethernet_GetMacAddress((ui8 *)&pucData[SSC_POS_MAC]);

		pucData[11] = (*(ui8 *)&SystemConfig.flags);
		pucData[12] = 0;

		pucData[13] = 24;

		LogicCopyEvents( (ui8 *)&pucData[14] );

		if ( 0xFFFFFFFF == addr->addr )
		{
			// Broadcast
			addr = IP_ADDR_BROADCAST;
		}
		else
		{
			// Unicast
		}

		udp_sendto(UdpControlPort, p, addr, SSC_UDP_PORT_TX);

		//
		// Free the pbuf.
		//
		pbuf_free(p);
	}
}

//*****************************************************************************
//
// SSC_UpdatePwmDuty - Processes a PWM Duty message
//
//*****************************************************************************
void SSC_UpdatePwmDuty( ui8 *buffer )
{
volatile float scaledFreqFloat;
ui8 bitFlags = buffer[1];
ui8 mask = 0;
ui8 i = 0;
ui16 *pwmDuty;

	if ( bitFlags & BIT7 )
	{
		scaledFreqFloat = pwmGetFreq();
	}

	for (i=0; i<7; i++)
	{
		mask = (0x01 << i);
		if ( bitFlags & mask )
		{
			// PWM duty for this channel will be Updated,
			// 2 is the offset in the message of the first PWM value
			pwmDuty = (ui16 *)&buffer[2+(i*2)];

			if ( bitFlags & BIT7 )
			{
				*pwmDuty = scaledFreqFloat * (float)((float)*pwmDuty / (float)100 );
			}

			pwmSetDuty( *pwmDuty, mask );
		}
	}
}

//*****************************************************************************
//
// SSC_SplashPixelUpdate - Update for the SplashPixel Panel
//
//*****************************************************************************
void SSC_SplashPixelUpdate ( struct pbuf *p )
{
#ifdef SPLASHPIXEL_ENABLED
volatile ui8 *pucData;
struct pbuf *nextP;
ui16 messageLen;
ui16 messageOffset;

	// Long UDP messages arrive in chunks from lwip
	messageLen = p->tot_len;
	messageOffset = 0;

	pucData = p->payload;

	SP_CopyFrameBuffer(messageOffset, &pucData[4], p->len-4);

	messageOffset += p->len;

	nextP = p;

	while (( nextP->tot_len > nextP->len ) || ( messageOffset < messageLen))
	{
		// move to next buffer
		nextP = nextP->next;
		pucData = nextP->payload;
		SP_CopyFrameBuffer((messageOffset-4), &pucData[0], nextP->len);

		// Update total bytes processed
		messageOffset += nextP->len;
	}
#endif
}

//*****************************************************************************
//
// SSC_ProcessCommand - Process received UDP commands
//
//*****************************************************************************
static void SSC_ProcessCommand(struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr)
{
volatile ui8 *pucData;
ui16 tempInt;
ui32 tempLong;
ui32 tempLong2;

ui8 replyWithStatus = 0;

	// Action the Message, be decided what command was sent
    pucData = p->payload;
    
	switch( pucData[0] )
	{
		case SSC_PING :
			// Todo : check for hello ?
			replyWithStatus = true;
		break;
		
		case SSC_EXTENDED_PING :
			SSC_SendExtendedInfo( addr );
		break;

		case SSC_RELAY_CON :
			// bit fields to indicate which relays need to be on or off
			RelayControl( pucData[1], pucData[2]  );
		break;
			
		case SSC_PWM_DUTY_ALL :
			SSC_UpdatePwmDuty( (ui8 *)pucData );
		break;
		
		case SSC_PWM_FREQ :
			tempInt = (pucData[2] << 8) |  pucData[1];
			pwmSetFreq( tempInt,  0xFF );
		break;
		
		case SSC_PWM_COLOUR_MODE :
			tempLong = (pucData[2] << 16) | (pucData[3] << 8) | pucData[4];
			tempLong2 = (pucData[5] << 16) | (pucData[6] << 8) | pucData[7];
			tempInt = (pucData[10] << 8) | pucData[9];

			ColourModeSet(pucData[1], tempLong, tempLong2, tempInt, pucData[8]);		
		break;
	
		case SSC_SET_UNIT_NAME :
			SysSetBaseName( (ui8 *)&pucData[2] , pucData[1] );
			replyWithStatus = true;
		break;
		
		case SSC_SET_RELAY_NAME :
			// String length
			tempInt = pucData[2];

			SysSetRelayName ( (ui8 *)&pucData[3], pucData[2], pucData[1], false );
		break;

		case SSC_SET_CONFIG :

			if ( (*(ui8 *)&SystemConfig.flags) != pucData[1] )
			{
				// Settings changed!!
				SystemConfig.flags = (*((SysConfigFlags *)&pucData[1]));

				if (SystemConfig.flags.StaticIp)
				{
					// Now static, might have changed IP settings
					SystemConfig.ulStaticIP = htonl( *((ui32 *)&pucData[4]) );
					SystemConfig.ulSubnetMask = htonl( *((ui32 *)&pucData[8]) );
					SystemConfig.ulGatewayIP = htonl( *((ui32 *)&pucData[12]) );

					Ethernet_ReConfig();
				}
				else
				{
					// dynamic
					Ethernet_ReConfig();
				}

				RelayInit();
				pwmInit();
			}

			if ( SystemConfig.flags.StaticIp )
			{
				SystemConfig.ulStaticIP = htonl( *((ui32 *)&pucData[4]) );
				SystemConfig.ulSubnetMask = htonl( *((ui32 *)&pucData[8]) );
				SystemConfig.ulGatewayIP = htonl( *((ui32 *)&pucData[12]) );

				Ethernet_ReConfig();
			}

			SystemConfig.timeOffset = *((ui16 *)&pucData[51]);

			// Set the SNTP Address, which saves the config as well
			SysSetSntpAddress((ui8 *)&pucData[20], 31);

		break;

		case SSC_SAVE_CONFIG :
			SysConfigSave();
		break;


		case SSC_RESET :
			if (('k' == pucData[1]) && ('i' == pucData[2]) && ('c' == pucData[3]) && ('k' == pucData[4]))
			{
				// We have been told to reboot, pull the trigger
				HWREG(NVIC_APINT) = (NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ);
			}
			else if (('r' == pucData[1]) && ('e' == pucData[2]) && ('f' == pucData[3]) && ('l' == pucData[4]))
			{
				UpdateFirmwareReq();
			}
			else if (('d' == pucData[1]) && ('e' == pucData[2]) && ('f' == pucData[3]) && ('a' == pucData[4]))
			{
				SysConfigFactoryDefault();

				SysConfigSave();

				// Reboot
				HWREG(NVIC_APINT) = (NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ);
			}
		break;
		
		case SSC_MANUAL_GPIO_DIR :
			// Port, Mask, Data
			UserGpioDirection( pucData[1], (*(ui16 *)&pucData[2]), (*(ui16 *)&pucData[6]) );
			SSC_SendAllPortInfo( addr );
		break;

		case SSC_MANUAL_GPIO_DATA :
			// Port, Mask, Data
			UserGpioSetOutputs( pucData[1], (*(ui16 *)&pucData[2]), (*(ui16 *)&pucData[6]) );
			SSC_SendAllPortInfo( addr );
		break;

		case SSC_MANUAL_GPIO_CONF :
			UserGpioDirection( pucData[1], 0xffff, (*(ui16 *)&pucData[4]) );
			UserGpioSetOutputs( pucData[1], 0xffff, (*(ui16 *)&pucData[6]) );
			SSC_SendAllPortInfo( addr );
		break;

		case SSC_GET_ALL_GPIO :
			SSC_SendAllPortInfo( addr );
		break;

		case SSC_INIT_GPIO_CONF :
			if ( pucData[1] < GPIO_PORT_TOTAL )
			{
				SystemConfig.UserGpioInit[pucData[1]][0] = (*(ui16 *)&pucData[4]);
				SystemConfig.UserGpioInit[pucData[1]][1] = (*(ui16 *)&pucData[6]);
				SSC_SendAllPortInfo( addr );
				// Remeber to save your changes!
			}
		break;

		case SSC_INIT_GPIO_RUN :
			UserGpioInit();
			SSC_SendAllPortInfo( addr );
		break;

		case SSC_LOGIC_EDT_ACT :
			LogicEditAction( pucData[1], (ui8 *)&pucData[2]);
		break;

		case SSC_LOGIC_EDT_COND :
			LogicEditCondition( pucData[1], (ui8 *)&pucData[2]);
		break;

		case SSC_LOGIC_EDT_EVENT :
			LogicEditEvent( pucData[2], (ui8 *)&pucData[3]);
		break;

		case SSC_LOGIC_READ_ACTION :
			SSC_SendLogicActions( addr );
		break;

		case SSC_LOGIC_READ_CONDTION :
			SSC_SendLogicConditions( addr );
		break;

		case SSC_LOGIC_READ_EVENTS :
			SSC_SendLogicEvents( addr );
		break;

		case SSC_BRIDGE_SCAN :
			// Kick off the scan. On SPI and I2C
			SolderBridge_StartScan();
			ExtGpio_Scan();
		break;

		case SSC_SB_SERVOPOS :
			SB_ServoSet(0xFF, (ui8 *)&pucData[4], pucData[2], pucData[3]);
		break;

		case SSC_SPLASHPIXEL_FBSET :
			SSC_SplashPixelUpdate( p );
		break;

		default :
		
		break;
	}
	
	pbuf_free(p);
	
	if ( replyWithStatus )
	{
		SSC_SendReply(addr);
	}
	
}

//*****************************************************************************
// SSC_Recieve
// This function is called by the lwIP TCP/IP stack when it receives a UDP
// packet from the RX port.  It actions it and produces the response packet, which is
// sent back to the client.
//
//*****************************************************************************
static void SSC_Recieve(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
	// the supplied p buf will indicate if there is more than one message buffer
	if ( p->len != p->tot_len )
    {
    	
    }
    
    // Process this message
    SSC_ProcessCommand(pcb, p, addr);
    
    //
    // The incoming pbuf is no longer needed, so free it.
    //
    pbuf_free(p);
}


