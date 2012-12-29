/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2012-02-26 - www.soldersplash.co.uk - C. Matthews - R. Steel

 This work is licensed under a Creative Commons Attribution-ShareAlike 3.0 Unported License.

*/

#ifdef _25AA02E48_

// Eeprom Commands - Page 7 of the Datasheet
#define EEPROM_CMD_READ			0x03
#define EEPROM_CMD_WRITE		0x02
#define EEPROM_CMD_RESET_WL		0x04
#define EEPROM_CMD_SET_WL		0x06
#define EEPROM_CMD_READ_ST		0x05		// Read status
#define EEPROM_CMD_WRITE_ST		0x01		// Write status

#define EEPROM_MAC_LOCATION		0xFA		// Eeprom address for the start of the MAC address

#define MICROCHIP_OUI_0			0x00
#define MICROCHIP_OUI_1			0x04
#define MICROCHIP_OUI_2			0xA3

#endif

bool Eeprom_ReadMacAddr( ui8 *macAddressBuffer );
ui8 Eeprom_GetStatus (void);
void Eeprom_SpiInit( void );
