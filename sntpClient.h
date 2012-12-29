
#define UNIX_EPOCH					2208988800UL			// this is the difference between 1900 (NTP Epoch) and 1970, the Unix Epoc


/* SNTP protocol defines */
#define SNTP_PORT                   123
#define SNTP_MAX_DATA_LEN           48
#define SNTP_RCV_TIME_OFS           32
#define SNTP_LI_NO_WARNING          0x00
#define SNTP_VERSION               (4/* NTP Version 4*/<<3)
#define SNTP_MODE_CLIENT            0x03
#define SNTP_MODE_SERVER            0x04
#define SNTP_MODE_BROADCAST         0x05
#define SNTP_MODE_MASK              0x07

void SntpInit( void );
void SntpGetTime( void );
static void SntpRecieve(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
