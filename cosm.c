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
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_nvic.h"
#include "utils/lwiplib.h"
#include "utils/locator.h"
#include "lwip/dns.h"

#include <stdio.h>

#include "datatypes.h"

#include "adcControl.h"

#define COSM_HOST 	"api.cosm.com"

struct ip_addr CosmIpAddress;

void CosmDisconnect(struct tcp_pcb *pcb);
err_t CosmDataHasGone (void *arg, struct tcp_pcb *pcb, u16_t len);
void CosmOnline ( void *arg, struct tcp_pcb *pcb, err_t err );
void CosmTest(unsigned long data);
void CosmGetIp (void);
void CosmServerFound (const char *name, struct ip_addr *ipaddr, void *arg);

void CosmServerFound (const char *name, struct ip_addr *ipaddr, void *arg)
{
	if ((ipaddr) && (ipaddr->addr))
	{
		CosmIpAddress.addr = ipaddr->addr;
		CosmTest(1);
	}
	else
	{

	}
}

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

char CosmCommandBuffer[255];
unsigned long CosmCommandLen;

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

	dataLen = sprintf(dataBuffer, "ADC0,%u\r\nADC1,%u\r\nTemperature,%u\r\n", adc0, adc1, temperature );
	dataLen = dataLen-6;

	//Authenticate
	CosmCommandLen = sprintf(&CosmCommandBuffer[0], "PUT /v2/feeds/83864.csv HTTP/1.1\r\n");
	CosmCommandLen += sprintf(&CosmCommandBuffer[CosmCommandLen], "Host: api.cosm.com\r\n");
	CosmCommandLen += sprintf(&CosmCommandBuffer[CosmCommandLen], "X-ApiKey: 2mDYswqyT1UA5CCV9KrkZJhpMA6SAKw0VXNxYzNpa3pQQT0g\r\n");
	CosmCommandLen += sprintf(&CosmCommandBuffer[CosmCommandLen], "User-Agent: SplashBase\r\n");
	CosmCommandLen += sprintf(&CosmCommandBuffer[CosmCommandLen], "Content-Length: %u\r\n", dataLen);
	CosmCommandLen += sprintf(&CosmCommandBuffer[CosmCommandLen], "Content-Type: text/csv\r\n");
	CosmCommandLen += sprintf(&CosmCommandBuffer[CosmCommandLen], "Connection: close\r\n\r\n");
	CosmCommandLen += sprintf(&CosmCommandBuffer[CosmCommandLen], "%s", dataBuffer);
	//CosmCommandLen += sprintf(&CosmCommandBuffer[CosmCommandLen], "ADC1,%d\r\n");

	// Open up a socket
	pcb=tcp_new();
	tcp_bind(pcb, IP_ADDR_ANY, 1223);
	tcp_connect(pcb, &CosmIpAddress, 80, CosmOnline);
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
