
void CosmDisconnect(struct tcp_pcb *pcb);
err_t CosmDataHasGone (void *arg, struct tcp_pcb *pcb, u16_t len);
err_t CosmOnline ( void *arg, struct tcp_pcb *pcb, err_t err );
void CosmTest(unsigned long data);
void CosmGetIp (void);
void CosmServerFound (const char *name, struct ip_addr *ipaddr, void *arg);
void CosmHostUpdated ( void );
