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
#define EXT_GPIO_INTTERUPT_PORT			GPIO_PORTD_BASE
#define EXT_GPIO_INTTERUPT_PIN			BIT7

typedef struct PCA9555
{
	ui8 pcaAvailible;				// Mask to show whcih pca's we have detected

	ui16 direction[ PCA_MAX ];
	ui16 input[ PCA_MAX ];
	ui16 output[ PCA_MAX ];
} PCA9555;

#ifdef _EXT_GPIO_

PCA9555 IoExpanders;

#else

extern PCA9555 IoExpanders;

#endif

void ExtGpio_Scan ( void );
void ExtGpio_UpdateInputs ( void );
bool ExtGpio_ReadPort ( ui8 address, ui16 *portValue );
