/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2013.03 - www.soldersplash.co.uk - C. Matthews - R. Steel

 Used when SplashPixel Code included, adds SPI interrupts

*/

// Based on Stellarisware example

#include "globals.h"

#include "datatypes.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ssi.h"
#include "inc/hw_types.h"
#include "driverlib/ssi.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "utils/uartstdio.h"

// TODO : Remove this include, have it register a function to call on interupt
#include "SplashPixel.h"

void SSI_IntHandler( void )
{
	SSIIntClear(SSI0_BASE, SSI_TXFF);
	
#ifdef SPLASHPIXEL_ENABLED
	if (SP_FeedTheFifo(false))
	{
		// more to send	
	}
	else
	{
		//empty	
		SSIIntDisable(SSI0_BASE, SSI_TXFF);
	}
#else
	SSIIntDisable(SSI0_BASE, SSI_TXFF);
#endif
	
}

void SSI_TxFifoIntEnable( void )
{
	SSIIntEnable(SSI0_BASE, SSI_TXFF);  	
}

void SPI_Init( void )
{
	ui32 tempLong;
	
	// Configure the SPI, 12bit opperation
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	
	// Enable SPI 0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);	
	
	GPIOPinTypeSSI(GPIO_PORTA_BASE, (GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_3 | GPIO_PIN_2));
                   
    GPIOPadConfigSet(GPIO_PORTA_BASE, (GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_3 | GPIO_PIN_2), GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);
                   
    // Configure and enable the SSI port for SPI master mode.  Use SSI0,
    // system clock supply, idle clock level low and active low clock in
    // freescale SPI mode, master mode, 1MHz SSI frequency, and 8-bit data.
    // For SPI mode, you can set the polarity of the SSI clock when the SSI
    // unit is idle.  You can also configure what clock edge you want to
    // capture data on.  Please reference the datasheet for more information on
    // the different SPI modes.
    //
    SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 1000000, 12);
    //SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 10000, 12);
                       
	SSIEnable(SSI0_BASE);
	
	//
    // Read any residual data from the SSI port.  This makes sure the receive
    // FIFOs are empty, so we don't read any unwanted junk.  This is done here
    // because the SPI SSI mode is full-duplex, which allows you to send and
    // receive at the same time.  The SSIDataGetNonBlocking function returns
    // "true" when data was returned, and "false" when no data was returned.
    // The "non-blocking" function checks if there is any data in the receive
    // FIFO and does not "hang" if there isn't.
    //
    while(SSIDataGetNonBlocking(SSI0_BASE, &tempLong))
    {
    }        
    
    SSIIntRegister(SSI0_BASE, &SSI_IntHandler);
    SSIIntEnable(SSI0_BASE, SSI_TXFF);      
}

bool SPI_AddToFifo( ui32 data )
{
	return( SSIDataPutNonBlocking(SSI0_BASE, data) );
}

bool SPI_Busy( void )
{
	if(	SSIBusy(SSI0_BASE) )
	{
		return( true );
	}
	else
	{
		return( false );	
	}	
}
