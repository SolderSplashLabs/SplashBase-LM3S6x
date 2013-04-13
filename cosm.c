/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2013 - www.soldersplash.co.uk - C. Matthews - R. Steel

*/
#include <string.h>
#include "SplashBaseHeaders.h"
#include "lwip/tcp.h"

struct ip_addr CosmIpAddress;
char CosmCommandBuffer[300];
unsigned long CosmCommandLen;


// *****************************************************************************
// CosmHostUpdated
// Host Has changed
// *****************************************************************************
void CosmHostUpdated ( void )
{
	CosmIpAddress.addr = 0;
}

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
		if (ERR_OK == dns_gethostbyname((const char*)SystemConfig.cosmHost, &CosmIpAddress, CosmServerFound, NULL))
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
	err_t error;

	adc0 = AdcGetResult(0);
	adc1 = AdcGetResult(1);
	temperature = AdcGetTemperature();

	dataLen = usprintf(dataBuffer, "ADC0,%u\r\nADC1,%u\r\nTemperature,%u\r\n", adc0, adc1, temperature );

	// /v2/feeds/%s.csv
	CosmCommandLen = usprintf(&CosmCommandBuffer[0], "PUT %s HTTP/1.1\r\n", SystemConfig.cosmUrl );
	CosmCommandLen += usprintf(&CosmCommandBuffer[CosmCommandLen], "Host: %s\r\nUser-Agent: SplashBase\r\n", SystemConfig.cosmHost );
	CosmCommandLen += usprintf(&CosmCommandBuffer[CosmCommandLen], "X-ApiKey: %s\r\n", SystemConfig.cosmPrivKey);
	CosmCommandLen += usprintf(&CosmCommandBuffer[CosmCommandLen], "Content-Type: text/csv\r\nContent-Length: %u\r\n", dataLen);
	CosmCommandLen += usprintf(&CosmCommandBuffer[CosmCommandLen], "Connection: close\r\n\r\n");

	// Now upload the data
	CosmCommandLen += usprintf(&CosmCommandBuffer[CosmCommandLen], "%s", dataBuffer);

	// Open up a socket
	pcb = tcp_new();

	if ( pcb != 0 )
	{
		tcp_bind(pcb, IP_ADDR_ANY, 0);
		error = tcp_connect(pcb, (struct ip_addr *)&CosmIpAddress, 80, CosmOnline);

		if ( ERR_OK == error )
		{
			// Success!
			UARTprintf("Cosm Connecting ...");
		}
		else
		{
			UARTprintf("Cosm Connect Error : %u", error);
		}
	}
	else
	{
		UARTprintf("Cosm No Memory To Create Socket");
	}
}

err_t CosmOnline ( void *arg, struct tcp_pcb *pcb, err_t err )
{
	// We have a connection, come in over, do you read me!
	LWIP_UNUSED_ARG(arg);

	if (err != ERR_OK)
	{
		// Some thing is wrong
		UARTprintf("Cosm Error : %u", err);
	}
	else
	{
		tcp_sent(pcb, CosmDataHasGone);

		// Transmit the string
		UARTprintf("Socket Open, Sending ... \n");
		tcp_write(pcb, CosmCommandBuffer, CosmCommandLen, 0);
		UARTprintf("%s\n", &CosmCommandBuffer[0]);

		//tcp_recv(pcb, server_recv);
	}

	return ERR_OK;
}

err_t CosmDataHasGone (void *arg, struct tcp_pcb *pcb, u16_t len)
{
	LWIP_UNUSED_ARG(arg);

	UARTprintf("Data sent to COSM. Closing Socket.\n");
	CosmDisconnect(pcb);
	return ERR_OK;
}

void CosmDisconnect(struct tcp_pcb *pcb)
{
	tcp_arg(pcb, NULL);
	tcp_sent(pcb, NULL);
	tcp_close(pcb);
}
