/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2012 - www.soldersplash.co.uk - C. Matthews - R. Steel

*/
#include <string.h>
#include "SplashBaseHeaders.h"

struct ip_addr CosmIpAddress;
char CosmCommandBuffer[255];
unsigned long CosmCommandLen;

// *****************************************************************************
// CosmServerFound
// DNS Resolve has found us an IP
// *****************************************************************************
void CosmServerFound (const char *name, struct ip_addr *ipaddr, void *arg)
{
	if ((ipaddr) && (ipaddr->addr))
	{
		CosmIpAddress.addr = ipaddr->addr;
		CosmTest(1);
	}
	else
	{
		// No Ip found
	}
}

// *****************************************************************************
// CosmGetIp
// Check to see if we need an IP for cosm
// *****************************************************************************
void CosmGetIp ( void )
{
	// resolve the DNS if needed ...
	if ((CosmIpAddress.addr == 0) || (CosmIpAddress.addr == 0xFFFFFFFF))
	{
		if (ERR_OK == dns_gethostbyname(COSM_HOST, &CosmIpAddress, CosmServerFound, NULL))
		{
			// It was cached
			CosmTest(1);
		}
	}
	else
	{
		// Not needed we have it
		CosmTest(1);
	}
}

// *****************************************************************************
// CosmTest
// Upload a result to COSM
// *****************************************************************************
void CosmTest(unsigned long data)
{
	struct tcp_pcb *pcb;
	char dataBuffer[50];
	volatile int dataLen;
	volatile unsigned int adc0 = 0;
	volatile unsigned int adc1 = 0;
	volatile unsigned int temperature = 0;

	adc0 = AdcGetResult(0);
	adc1 = AdcGetResult(1);
	temperature = AdcGetTemperature();

	dataLen = usprintf(dataBuffer, "ADC0,%u\r\nADC1,%u\r\nTemperature,%u\r\n", adc0, adc1, temperature );
	dataLen = dataLen-6;

	//Authenticate
	CosmCommandLen = usprintf(&CosmCommandBuffer[0], "PUT /v2/feeds/%s.csv HTTP/1.1\r\n Host: api.cosm.com\r\nX-ApiKey: ", COMS_FEED_ID);
	// Give them our secret key
	CosmCommandLen += usprintf(&CosmCommandBuffer[CosmCommandLen], COSM_PRIV_KEY);
	// Tell them how much data you wish to send
	CosmCommandLen += usprintf(&CosmCommandBuffer[CosmCommandLen], "\r\nUser-Agent: SplashBase\r\nContent-Length: %u\r\nContent-Type: text/csv\r\n", dataLen);
	CosmCommandLen += usprintf(&CosmCommandBuffer[CosmCommandLen], "Connection: close\r\n\r\n");
	// Now upload the data
	CosmCommandLen += usprintf(&CosmCommandBuffer[CosmCommandLen], "%s", dataBuffer);

	// Open up a socket
	pcb=tcp_new();
	tcp_bind(pcb, IP_ADDR_ANY, 1223);
	if ( ERR_OK == tcp_connect(pcb, (struct ip_addr *)&CosmIpAddress, 80, CosmOnline) )
	{
		// Success!
	}
}

void CosmOnline ( void *arg, struct tcp_pcb *pcb, err_t err )
{
	// We have a connection, come in over, do you read me!
	LWIP_UNUSED_ARG(arg);

	if (err != ERR_OK)
	{
		// Some thing is wrong
	}
	else
	{
		tcp_sent(pcb, CosmDataHasGone);

		// Transmit the string
		tcp_write(pcb, CosmCommandBuffer, CosmCommandLen, 0);

		//tcp_recv(pcb, server_recv);
	}
}

err_t CosmDataHasGone (void *arg, struct tcp_pcb *pcb, u16_t len)
{
	LWIP_UNUSED_ARG(arg);
	CosmDisconnect(pcb);
	return ERR_OK;
}

void CosmDisconnect(struct tcp_pcb *pcb)
{
	tcp_arg(pcb, NULL);
	tcp_sent(pcb, NULL);
	tcp_close(pcb);
}
