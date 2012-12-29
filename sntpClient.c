/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2012 - www.soldersplash.co.uk - C. Matthews - R. Steel

RFC5905 - Details the NTPv4 Protocol

*/
#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_nvic.h"

#include "lwiplib.h"				// lwip TCP/IP Stack
#include "lwip/dns.h"

#include "datatypes.h"
#include "globals.h"

#include "time.h"
#include "sntpClient.h"

struct udp_pcb *SntpPort;
struct ip_addr SntpServerIpAddr;
unsigned long SntpTimeStamp = 0;

// *****************************************************************************
// SntpSendPacket
//
//
// *****************************************************************************
void SntpSendPacket ( void )
{
struct pbuf *p;
volatile u8_t *pucData;
u8_t i;
u8_t sntp_request [SNTP_MAX_DATA_LEN];

	//
	// Allocate a new pbuf for sending the response.
	//
	p = pbuf_alloc(PBUF_TRANSPORT, SNTP_MAX_DATA_LEN, PBUF_RAM);
	if(p == NULL)
	{
		return;
	}

	memset(sntp_request, 0, SNTP_MAX_DATA_LEN);

	sntp_request[0] = SNTP_LI_NO_WARNING | SNTP_VERSION | SNTP_MODE_CLIENT;
	sntp_request[1] = 0;     // Stratum, or type of clock
	sntp_request[2] = 6;     // Polling Interval
	sntp_request[3] = 0xEC;  // Peer Clock Precision
	  // 8 bytes of zero for Root Delay & Root Dispersion

	// Reference ID?
	sntp_request[12]  = 49;
	sntp_request[13]  = 0x4E;
	sntp_request[14]  = 49;
	sntp_request[15]  = 52;

	// Fill the array with data ..
	pucData = p->payload;

	//
	// Copy the response packet data into the pbuf.
	//
	for(i = 0; i < sizeof(sntp_request); i++)
	{
		pucData[i] = sntp_request[i];
	}

	//
	// Send the response.
	//
	udp_sendto(SntpPort, p, &SntpServerIpAddr, SNTP_PORT);

	//
	// Free the pbuf.
	//
	pbuf_free(p);

}

// *****************************************************************************
// SntpServerFound
//
//
// *****************************************************************************
void SntpServerFound (const char *name, struct ip_addr *ipaddr, void *arg)
{
	if ((ipaddr) && (ipaddr->addr))
	{
		// Ip should have been copied to SntpServerIpAddr
		SntpServerIpAddr.addr = ipaddr->addr;

		SntpSendPacket();
	}
	else
	{
		// failed, fallback to ip?
		SntpServerIpAddr.addr = inet_addr("89.238.66.126");
		SntpSendPacket();

		// Try DNS next time
		SntpServerIpAddr.addr = 0;
	}
}

// *****************************************************************************
// SntpGetTime
//
//
// *****************************************************************************
void SntpGetTime( void )
{
	// First resolve the DNS if needed ...
	if (SntpServerIpAddr.addr == 0)
	{
		dns_gethostbyname("0.pool.ntp.org", &SntpServerIpAddr, SntpServerFound, NULL);
	}
	else
	{
		// Send SNTP
		SntpSendPacket();
	}
}

// *****************************************************************************
// SntpRecieve
// This function is called by the lwIP TCP/IP stack when it receives a UDP
// RFC-1305 defines the format of the response packet
//
// *****************************************************************************
static void SntpRecieve(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
volatile u8_t sntp_response[SNTP_MAX_DATA_LEN];
u8_t i = 0;
char *data;
volatile unsigned long timestamp = 0;

	data = p->payload;

	// TODO : dont need to do this, it's just so we can inspect the data, remove
	for (i=0; i< p->len; i++)
	{
		sntp_response[i] = data[i];
	}

	// Transmit Timestamp starts at byte 40
	// Quick and dirty method, take the transmit time of the packet and use that as our clock time
	// Note : This does not take into account, Internet delays or code delays at all!!
	timestamp = data[40];
	timestamp <<= 8;
	timestamp |= data[41];
	timestamp <<= 8;
	timestamp |= data[42];
	timestamp <<= 8;
	timestamp |= data[43];

	// Timestamp now contains the current no of seconds from 1900, lets convert it to the unix version of 1970
	timestamp -= 2208988800UL;

	// instead of being always slow add a second on so that were always fast
	timestamp ++;
	Time_SetUnix(timestamp);

	SntpTimeStamp = timestamp;

    //
    // The incoming pbuf is no longer needed, so free it.
    //
    pbuf_free(p);
}

// *****************************************************************************
// SntpInit
// Tells lWip to open a port and what function to call upon recv
//
// *****************************************************************************
void SntpInit( void )
{
    //
    // Create a new UDP port for listening for commands.
    //
	SntpPort = udp_new();
	udp_recv(SntpPort, SntpRecieve, NULL);
    udp_bind(SntpPort, IP_ADDR_ANY, SNTP_PORT);
}
