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

typedef enum USER_GPIO_PORTS
{
	USER_GPIO_PORTA = 0,
	USER_GPIO_PORTB,
	USER_GPIO_PORTC,
	USER_GPIO_PORTD,
	USER_GPIO_PORTE,
	USER_GPIO_PORTF,
	USER_GPIO_PORTG,

	USER_GPIO_PORTH_EXT,
	USER_GPIO_PORTI_EXT,
	USER_GPIO_PORTJ_EXT,
	USER_GPIO_PORTK_EXT,
	USER_GPIO_PORTL_EXT,
	USER_GPIO_PORTM_EXT,
	USER_GPIO_PORTN_EXT,
	USER_GPIO_PORTO_EXT
} USER_GPIO_PORTS;

#ifdef _USER_GPIO_
const ui32 GPIO_REGISTERS[] = { GPIO_PORTA_BASE, GPIO_PORTB_BASE, GPIO_PORTC_BASE, GPIO_PORTD_BASE, GPIO_PORTE_BASE, GPIO_PORTF_BASE, GPIO_PORTG_BASE };
const ui8 GPIO_PORT_LETTERS[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O' };
#else

extern const ui32 GPIO_REGISTERS[];
extern const ui8 GPIO_PORT_LETTERS[];

#endif

void UserGpioInit ( void );
ui32 UserGpio_AppGetMask( ui8 portNo, ui32 mask );
void UserGpio_AppSetMask( ui8 portNo, ui32 mask );
bool UserGpioDirInput ( ui8 portNo, ui32 pins );
bool UserGpioDirOutput ( ui8 portNo, ui32 pins );
bool UserGpioSetOutputs ( ui8 portNo, ui32 mask, ui32 newVal);
bool UserGpioGet ( ui8 portNo, ui32 *buffer );
bool UserGpioDirGet ( ui8 portNo, ui32 *buffer );
void ExtGpio_GetPort ( ui8 port, ui16 *buffer );
bool UserGpioDirection ( ui8 portNo, ui32 mask, ui32 pins );

