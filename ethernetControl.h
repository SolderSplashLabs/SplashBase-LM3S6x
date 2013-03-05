// if DHCP fails
// Default IP is 192.168.13.13
#define ETH_DEFAULT_IP0		192
#define ETH_DEFAULT_IP1		168
#define ETH_DEFAULT_IP2		13
#define ETH_DEFAULT_IP3		13

#define ETH_DEFAULT_MASK0	255
#define ETH_DEFAULT_MASK1	255
#define ETH_DEFAULT_MASK2	0
#define ETH_DEFAULT_MASK3	0


void Ethernet_Task ( void );
void Ethernet_Init ( void );
bool Ethernet_Connected( void );
void Ethernet_GetMacAddress ( ui8 *pBuf );
ui32 Ethernet_GetIp ( void );
ui32 Ethernet_GetGatewayIp ( void );
ui32 Ethernet_GetNetmask ( void );
void Ethernet_ReConfig ( void );

