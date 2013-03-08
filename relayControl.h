#ifdef RELAYCONTROL

ui8 RelaysClosed = 0x00;
ui8 RelayStatus = false;


#if SPLASHBASE_BOARD_REV == 2
	#define RELAY_1_BIT	BIT0
	#define RELAY_2_BIT	BIT1
	#define RELAY_3_BIT	BIT4
	#define RELAY_4_BIT	BIT5
#else
	#define RELAY_1_BIT	BIT0
	#define RELAY_2_BIT	BIT1
	#define RELAY_3_BIT	BIT2
	#define RELAY_4_BIT	BIT3
#endif

#define RELAY_OUT_BITS	 (RELAY_1_BIT | RELAY_2_BIT | RELAY_3_BIT | RELAY_4_BIT)


#endif 

extern ui8 RelayStatus;
//extern ui8 RelaysClosed;

void RelayInit( void );
void RelayControl( ui8 closedRelays, ui8 mask );
void relaysOn( void );
void relaysOff( void );
ui8 RelayGetClosed( void );


