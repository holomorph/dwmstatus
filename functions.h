/* See LICENSE file for copyright and license details. */

typedef struct {
	const char *name;
	char *rx;
	char *tx;
	long rx_bytes;
	long tx_bytes;
} Interface;

/* utility functions */
char *smprintf(const char *fmt, ...) __attribute__((format (printf, 1, 2)));
int runevery(time_t *ltime, int sec);

/* control abstractions */
int network_init(Interface *iface, const char *ifname);
void network_deinit(Interface *iface);

/* status elements */
char *loadavg(void);
char *coretemp(void);
char *memory(void);
char *network(Interface *iface, long rx_old, long tx_old);
char *ipaddr(const char *ifname);
char *battery(void);
char *mktimes(int tmsleep);
char *new_mail(const char *maildir);
