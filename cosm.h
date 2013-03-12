
// TODO : Moved to config .CosmPrivKey
//#define COSM_PRIV_KEY 	"2mDYswqyT1UA5CCV9KrkZJhpMA6SAKw0VXNxYzNpa3pQQT0g"
#define COSM_HOST		"api.cosm.com"
//#define COMS_FEED_ID	"83864"

void CosmDisconnect(struct tcp_pcb *pcb);
err_t CosmDataHasGone (void *arg, struct tcp_pcb *pcb, u16_t len);
void CosmOnline ( void *arg, struct tcp_pcb *pcb, err_t err );
void CosmTest(unsigned long data);
void CosmGetIp (void);
void CosmServerFound (const char *name, struct ip_addr *ipaddr, void *arg);
void CosmHostUpdated ( void );
