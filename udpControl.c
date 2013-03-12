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

/*
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"	
#include "inc/hw_nvic.h"
#include "lwiplib.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

#include "datatypes.h"
#include "globals.h"

#include "config.h"

#include "relayControl.h"
#include "pwmControl.h"
#include "colourModes.h"
#include "logicController.h"
#include "solderBridge\solderBridgeSpi.h"
#include "userGpioControl.h"
#include "ethernetControl.h"

#define UDPCONTROL
#include "udpControl.h"
*/


//*****************************************************************************
//
//! Initialises the Control service.
//!
//! This function prepares the locator service to handle device discovery
//! requests.  A UDP server is created and the locator response data is
//! initialized to all empty.
//!
//! \return None.
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
//! Sets the MAC address in the locator response packet.
//!
//! \param pucMACArray is the MAC address of the network interface.
//!
//! This function sets the MAC address of the network interface in the locator
//! response packet.
//!
//! \return None.
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
// SSC_SetUnitName
// Sets the unit name that is used when talking to the network
//
//*****************************************************************************
void SSC_SetUnitName(const ui8 *pcAppTitle)
{
ui8 strLenCnt = 0;

	// Todo : Magic numbers ...

	// Loop until you hit the max string length or find a 0 in the string
	for(strLenCnt = 0; (strLenCnt < 20) && *pcAppTitle; strLenCnt++)
	{
		SscReplyBuffer[strLenCnt + 31] = *pcAppTitle++;
	}

	SscReplyBuffer[30] = strLenCnt;

	//
	// Zero-fill the remainder of the space in the response data (if any).
	//
	for(; strLenCnt < 20; strLenCnt++)
	{
		SscReplyBuffer[strLenCnt + 31] = 0;
	}
}

//*****************************************************************************
// SSC_SetUnitName
// Sets the unit name that is used when talking to the network
//
//*****************************************************************************
void SSC_SetRelayName(const ui8 *relayName, ui8 relayNo)
{
ui8 strLenCnt = 0;
ui8 replyBufPos = 0;

	switch ( relayNo )
	{
		case 0 :
			replyBufPos = SSC_POS_RELAY1NAME;
		break;

		case 1 :
			replyBufPos = SSC_POS_RELAY2NAME;
		break;

		case 2 :
			replyBufPos = SSC_POS_RELAY3NAME;
		break;

		case 3 :
			replyBufPos = SSC_POS_RELAY4NAME;
		break;

		default :
			// Erm not a valid relay !
			return;
	}

	// Loop until you hit the max string length or find a 0 in the string
	for(strLenCnt = 0; (strLenCnt < SPLASHBASE_RELAYNAME_LEN) && *relayName; strLenCnt++)
	{
		SscReplyBuffer[strLenCnt + replyBufPos] = *relayName++;
	}

	//
	// Zero-fill the remainder of the space in the response data (if any).
	//
	for(; strLenCnt < SPLASHBASE_RELAYNAME_LEN; strLenCnt++)
	{
		SscReplyBuffer[strLenCnt + replyBufPos] = 0;
	}
}

//*****************************************************************************
// SSC_SendReply
// Sends the Boards Status out
//
//*****************************************************************************
static void SSC_SendReply(struct pbuf *p, struct ip_addr *addr)
{
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
    SscReplyBuffer[SSC_POS_STARTBYTE] = 0xE0;
    
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
    
    SscReplyBuffer[SSC_POS_STEPCNT] = 0x00FF & (ColourSettings.maxSteps >> 8);
    SscReplyBuffer[SSC_POS_STEPCNT + 1] = 0x00FF & ColourSettings.maxSteps;
    
    // Set the global on/off flags
    if (PwmStatus)
    {
    	SscReplyBuffer[SSC_POS_GLOBALOUT_BITS] |= BIT0;
    }
    else
    {
    	SscReplyBuffer[SSC_POS_GLOBALOUT_BITS] &= ~BIT0;
    }

	if (RelayStatus)
	{
    	SscReplyBuffer[SSC_POS_GLOBALOUT_BITS] |= BIT1;
    }
    else
    {
    	SscReplyBuffer[SSC_POS_GLOBALOUT_BITS] &= ~BIT1;
    }
    
	// Unit name already in the buffer

	// As our relay names

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

/*
bool SSC_CMD_RelayCon ( ui8 *buffer, ui8 len );
bool SSC_CMD_PwmDuty ( ui8 *buffer, ui8 len );
bool SSC_CMD_PwmDutyAll ( ui8 *buffer, ui8 len );
bool SSC_CMD_PwmFreq ( ui8 *buffer, ui8 len );
bool SSC_CMD_PwmDColorMode ( ui8 *buffer, ui8 len );
bool SSC_CMD_SetUnitName ( ui8 *buffer, ui8 len );
bool SSC_CMD_SetRelayName ( ui8 *buffer, ui8 len );
bool SSC_CMD_Reset ( ui8 *buffer, ui8 len );
bool SSC_CMD_OutputsOnOff ( ui8 *buffer, ui8 len );
bool SSC_CMD_GpioDir ( ui8 *buffer, ui8 len );
bool SSC_CMD_GpioData ( ui8 *buffer, ui8 len );
bool SSC_CMD_LogicAdd ( ui8 *buffer, ui8 len );
bool SSC_CMD_SetServoPos ( ui8 *buffer, ui8 len );


//*****************************************************************************
// SSC_ProcessCommand
// Process received UDP commands
//
//*****************************************************************************
static void SSC_ProcessCommand(struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr)
{
volatile ui8 *pucData;

	// UDP packet is already checksumed so packet should be valid

	pucData = p->payload;

	switch( pucData[0] )
	{
		case SSC_PING :
			SSC_CMD_RelayCon
		break;

		case SSC_RELAY_CON :

		break;

		case SSC_PWM_DUTY :

		break;

		case SSC_PWM_DUTY_ALL :

		break;

		case SSC_PWM_FREQ :

		break;

		case SSC_PWM_COLOUR_MODE :

		break;

		case SSC_SET_UNIT_NAME :

		break;

		case SSC_SET_RELAY_NAME :

		break;

		case SSC_RESET :

		break;

		case SSC_OUTPUTS_ON_OFF :

		break;

		case SSC_MANUAL_GPIO_DIR :

		break;

		case SSC_MANUAL_GPIO_DATA :

		break;

		case SSC_LOGIC_INSERT_CON :

		break;

		case SSC_SB_SERVOPOS :

		break;

		default :

		break;
	}

	pbuf_free(p);

	if ( replyWithStatus )
	{
		SSC_SendReply(p,addr);
	}

}
*/

void SSC_SendPortInfo ( struct ip_addr *addr, ui16 reason )
{
bool result = false;
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
// SSC_ProcessCommand
// Process received UDP commands
//
//*****************************************************************************
static void SSC_ProcessCommand(struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr)
{
volatile ui8 *pucData;
ui32 *ptLong;
ui32 *ptLong2;
ui16 tempInt;
ui16 tempInt2;
ui32 tempLong;
ui32 tempLong2;
ui16 tempDuty0;
ui16 tempDuty1;
ui16 tempDuty2;
volatile double scaledFreqFloat;
ui8 replyWithStatus = 0;
struct pbuf *nextP;


	//
	// Validate the contents of the datagram.
	//
    pucData = p->payload;
    
	switch( pucData[0] )
	{
		case SSC_PING :
			replyWithStatus = true;
		break;
		
		case SSC_RELAY_CON :
			// bit fields to indicate which relays need to be on or off
			RelayControl( pucData[1], pucData[2]  );
		break;
	
		case SSC_PWM_DUTY :
			// Byte1 MSB, Byte2 LSB, Byte3 Channel Mask, Byte4 Duty Scaled Flag
			// Set the duty of all PWM, to the supplied 16 bit value
			tempInt = (pucData[1] << 8) |  pucData[2];
			
			if ( pucData[4] )
			{

			}
			
			pwmSetDuty( tempInt, pucData[3] );
			
		break;
		
		case SSC_PWM_DUTY_ALL :
			// Byte1 MSB, Byte2 LSB, Byte3 Channel Mask, Byte4 Duty Scaled Flag
			// Set the duty of all PWM, to the supplied 16 bit value
			
			tempDuty0 = (pucData[1] << 8) |  pucData[2];
			tempDuty1 = (pucData[3] << 8) |  pucData[4];
			tempDuty2 = (pucData[5] << 8) |  pucData[6];
			
			if ( pucData[7] )
			{
				/*
				scaledFreqFloat = (float)( (float)pwmGetFreq() /  (float)100);
				tempDuty0 = (ui32)((float)tempDuty0 * scaledFreqFloat);
				tempDuty1 = (ui32)((float)tempDuty1 * scaledFreqFloat);
				tempDuty2 = (ui32)((float)tempDuty2 * scaledFreqFloat);
				*/

				scaledFreqFloat = pwmGetFreq();
				tempDuty0 = scaledFreqFloat * ((float)tempDuty0 / (float)100);
				tempDuty1 = scaledFreqFloat * ((float)tempDuty1 / (float)100);
				tempDuty2 = scaledFreqFloat * ((float)tempDuty2 / (float)100);

				/*
				scaledFreqFloat = 0xFFFF / pwmGetFreq();
				scaledFreqFloat *= 1000;
				
				tempDuty0 = (ui32)(tempDuty0 / scaledFreqFloat) * 1000;
				tempDuty1 = (ui32)(tempDuty1 / scaledFreqFloat) * 1000;
				tempDuty2 = (ui32)(tempDuty2 / scaledFreqFloat) * 1000;
				*/
			}
					
			pwmSetDuty( tempDuty0, 0x01 );
			pwmSetDuty( tempDuty1, 0x02 );
			pwmSetDuty( tempDuty2, 0x04 );			
			
		break;
		
		case SSC_PWM_FREQ :
			tempInt = (pucData[1] << 8) |  pucData[2];
			pwmSetFreq( tempInt,  0xFF );
		break;
		
		case SSC_PWM_COLOUR_MODE :
			tempLong = (pucData[2] << 16) | (pucData[3] << 8) | pucData[4];
			tempLong2 = (pucData[5] << 16) | (pucData[6] << 8) | pucData[7];
			tempInt = (pucData[9] << 8) | pucData[10];
			ColourModeSet(pucData[1], tempLong, tempLong2, tempInt, pucData[8]);		
		break;
	
		case SSC_SET_UNIT_NAME :
			SysSetBaseName( (ui8 *)&pucData[2] , pucData[1] );
		break;
		
		case SSC_SET_RELAY_NAME :
			// String length
			tempInt = pucData[2];

			if (tempInt > SPLASHBASE_RELAYNAME_LEN) tempInt = SPLASHBASE_RELAYNAME_LEN;

			switch (pucData[1])
			{
				case 0 :
					memcpy(SystemConfig.relayOneName, (char *)&pucData[3], tempInt);
					if (tempInt < SPLASHBASE_RELAYNAME_LEN) SystemConfig.relayOneName[tempInt] = 0;

					SSC_SetRelayName((ui8 *)SystemConfig.relayOneName, 0 );

				break;

				case 1 :
					memcpy(SystemConfig.relayTwoName, (char *)&pucData[3], tempInt);
					if (tempInt < SPLASHBASE_RELAYNAME_LEN) SystemConfig.relayTwoName[tempInt] = 0;

					SSC_SetRelayName((ui8 *)SystemConfig.relayTwoName, 1 );
				break;

				case 2 :
					memcpy(SystemConfig.relayThreeName, (char *)&pucData[3], tempInt);
					if (tempInt < SPLASHBASE_RELAYNAME_LEN) SystemConfig.relayThreeName[tempInt] = 0;

					SSC_SetRelayName((ui8 *)SystemConfig.relayThreeName, 2 );
				break;

				case 3 :
					memcpy(SystemConfig.relayFourName, (char *)&pucData[3], tempInt);
					if (tempInt < SPLASHBASE_RELAYNAME_LEN) SystemConfig.relayFourName[tempInt] = 0;

					SSC_SetRelayName((ui8 *)SystemConfig.relayFourName, 3 );
				break;
			}

			SysConfigSave();

		break;

		case SSC_RESET :
			if (('k' == pucData[1]) && ('i' == pucData[2]) && ('c' == pucData[3]) && ('k' == pucData[4]))
			{
				// We have been told to reboot, pull the trigger
				HWREG(NVIC_APINT) = (NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ);
			}
		break;
		
		case SSC_OUTPUTS_ON_OFF :
			if ( pucData[1] )
			{
				if ( pucData[2] & BIT0 ) pwmOn();
				if ( pucData[2] & BIT1 ) relaysOn();
			}
			else
			{
				if ( pucData[2] & BIT0 ) pwmOff();
				if ( pucData[2] & BIT1 ) relaysOff();
			}
		break;
		
		case SSC_MANUAL_GPIO_DIR :
			ptLong = (ui32 *)&pucData[2];
			ptLong2 = (ui32 *)&pucData[6];
			UserGpioDirection( pucData[1], *ptLong, *ptLong2 );
		break;

		case SSC_MANUAL_GPIO_DATA :
			ptLong = (ui32 *)&pucData[2];
			ptLong2 = (ui32 *)&pucData[6];
			UserGpioSetOutputs( pucData[1], *ptLong, *ptLong2 );
		break;

		case SSC_LOGIC_INSERT_CON :
			LogicInsertNewCondition( pucData[1], (ui8 *)&pucData[4] );
		break;

		case SSC_SB_SERVOPOS :
			SB_ServoSet(0xFF, (ui8 *)&pucData[4], pucData[2], pucData[3]);
		break;

		case SSC_SPLASHPIXEL_FBSET :

			#ifdef SPLASHPIXEL_ENABLED
			// Long UDP messages arrive in chunks from lwip
			tempInt = p->tot_len;
			tempInt2 = 0;

			SP_CopyFrameBuffer(tempInt2, &pucData[4], p->len-4);

			tempInt2 += p->len;

			nextP = p;

			while (( nextP->tot_len > nextP->len ) || ( tempInt2 < tempInt))
			{
				// move to next buffer
				nextP = nextP->next;
				pucData = nextP->payload;
				SP_CopyFrameBuffer((tempInt2-4), &pucData[0], nextP->len);

				// Update total bytes processed
				tempInt2 += nextP->len;
			}
			#endif

		break;

		default :
		
		break;
	}
	
	pbuf_free(p);
	
	if ( replyWithStatus )
	{
		SSC_SendReply(p,addr);
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
	//
	// Validate the contents of the datagram.
	//

	// the supplied p buf will indicate if theres more than one message buffer
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


