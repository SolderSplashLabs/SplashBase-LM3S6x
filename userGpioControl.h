/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2013 - www.soldersplash.co.uk - C. Matthews - R. Steel

*/

#define GPIO_PORT_MAX_MCU		7
#define GPIO_PORT_MAX_EXT		8
#define GPIO_PORT_TOTAL			GPIO_PORT_MAX_MCU + GPIO_PORT_MAX_EXT

#ifdef _USER_GPIO_
ui32 GPIO_REGISTERS[] = { GPIO_PORTA_BASE, GPIO_PORTB_BASE, GPIO_PORTC_BASE, GPIO_PORTD_BASE, GPIO_PORTE_BASE, GPIO_PORTF_BASE, GPIO_PORTG_BASE };
#else

extern ui32 GPIO_REGISTERS[];

bool UserGpioDirInput ( ui8 portNo, ui32 pins );
bool UserGpioDirOutput ( ui8 portNo, ui32 pins );
bool UserGpioSetOutputs ( ui8 portNo, ui32 mask, ui32 newVal);

#endif
