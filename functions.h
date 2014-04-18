/* See LICENSE file for copyright and license details. */

#include "util.h"

typedef struct {
	const char *name;
	char *rx;
	char *tx;
	long rx_bytes;
	long tx_bytes;
	long rx_old;
	long tx_old;
} Interface;

/* control abstractions */
void *network_init(const char *ifname);
void network_deinit(Interface *iface);

/* status elements */
void loadavg(buffer_t *buf);
void coretemp(buffer_t *buf);
void memory(buffer_t *buf);
void network(buffer_t *buf, Interface *iface);
void ipaddr(buffer_t *buf, const char *ifname);
void battery(buffer_t *buf);
void mktimes(buffer_t *buf, int *tmsleep);
void new_mail(buffer_t *buf, const char *maildir);

