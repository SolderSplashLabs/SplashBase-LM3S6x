/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2013 - www.soldersplash.co.uk - C. Matthews - R. Steel

 Redistributions of source code must retain the above copyright notice
*/

#define PCA_ADDR_BASE	0x20
#define PCA_MAX			8

#define PCA9555_REG_INPUT 		0
#define PCA9555_REG_OUPUT 		2
#define PCA9555_REG_POLARITY 	4
#define PCA9555_REG_DIR			6

#define EXT_GPIO_INTTERUPT_PORT			GPIO_PORTD_BASE
#define EXT_GPIO_INTTERUPT_PIN			BIT7

typedef struct PCA9555
{
	ui8 pcaAvailible;				// Mask to show which pca's we have detected
	ui8 updateOutput;				// Mask to show which PCA's outputs registers need updating
	ui8 updateDirection;			// Mask to show which PCA's direction registers need updating

	ui16 direction[ PCA_MAX ];
	ui16 input[ PCA_MAX ];
	ui16 output[ PCA_MAX ];
} PCA9555;

#ifdef _EXT_GPIO_

PCA9555 IoExpanders;

static bool ExtGpio_WriteReg ( ui8 address, ui8 reg, ui16 value );
static void ExtGpio_UpdateOutput( void );
static void ExtGpio_UpdateDirection( void );

#else

extern PCA9555 IoExpanders;

#endif

void ExtGpio_Task ( void );
void ExtGpio_Scan ( void );
void ExtGpio_UpdateInputs ( void );
bool ExtGpio_ReadPort ( ui8 address, ui16 *portValue );
void ExtGpio_SetPort ( ui8 port, ui16 mask, ui16 value );
void ExtGpio_SetDirection ( ui8 port, ui16 mask, ui16 dir );
