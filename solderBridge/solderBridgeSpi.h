/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2013-01-01 - www.soldersplash.co.uk - C. Matthews - R. Steel

 Redistributions of source code must retain the above copyright notice
*/

#define SB_SPI_CS_COUNT	5

#define SB_SPI_SLAVE0	GPIO_PIN_0
#define SB_SPI_SLAVE1	GPIO_PIN_1
#define SB_SPI_SLAVE2	GPIO_PIN_2
#define SB_SPI_SLAVE3	GPIO_PIN_3
#define SB_SPI_SLAVE4	GPIO_PIN_4

#define SB_SPI_CS_PORT	GPIO_PORTE_BASE
#define SB_SPI_CS_PINS	{(SB_SPI_SLAVE0 | SB_SPI_SLAVE1 | SB_SPI_SLAVE2 | SB_SPI_SLAVE3 | SB_SPI_SLAVE4);}

enum SB_TYPELIST
{
	SB_NONE,
	SB_SERVO_24,
	SB_DMX_MASTER,
	SB_NAUGHT_TO_TEN,
	SB_DATALOGGER,
	SB_NINE_DOF
};

void SB_ServoSet (ui8 slaveMask, ui8 *positions, ui8 servoOffset, ui8 servoCnt );


extern ui8 SolderBridgeList[ SB_SPI_CS_COUNT ];
