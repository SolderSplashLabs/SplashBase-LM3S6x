/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2012 - www.soldersplash.co.uk - C. Matthews - R. Steel

*/
// Port and Pin Defines used when enabling the UART
#define PIN_U0RX_PORT           GPIO_PORTA_BASE
#define PIN_U0RX_PIN            GPIO_PIN_0
#define PIN_U0TX_PORT           GPIO_PORTA_BASE
#define PIN_U0TX_PIN            GPIO_PIN_1

#define PIN_U1RX_PORT           GPIO_PORTD_BASE
#define PIN_U1RX_PIN            GPIO_PIN_2
#define PIN_U1TX_PORT           GPIO_PORTD_BASE
#define PIN_U1TX_PIN            GPIO_PIN_3

void Serial_Task ( void );
void Serial_Init ( void );
