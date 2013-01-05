/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2013 - www.soldersplash.co.uk - C. Matthews - R. Steel

 Redistributions of source code must retain the above copyright notice

 Globals should generally be avoided and data exchanged between software modules.
 But for key defines it makes sense
*/

// Number of times a second you would like the SysTick interrupt to fire
#define SYSTICKHZ               100

// How often in MS does the System tick occur
#define SYSTICKMS               (1000 / SYSTICKHZ)

#define SW_REV_MAJOR 			0
#define SW_REV_MINOR			6

// Revision of base board the software is running on
#define SPLASHBASE_BOARD_REV	2
//#define SPLASHBASE_BOARD_REV	1

// Define to enable HTTP
//#define HTTP_ENABLED

// Define to enable UPNP
//#define UPNP_ENABLED
// NOTE : You need to define LWIP_UPNP in lwipopts as well

#define SERIAL_ENABLED

