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
void network_deinit(Interface *iface);

/* status elements */
void loadavg(buffer_t *buf, void UNUSED *state);
void coretemp(buffer_t *buf, void UNUSED *state);
void memory(buffer_t *buf, void UNUSED *state);
void network(buffer_t *buf, void *state);
void ipaddr(buffer_t *buf, void *state);
void battery(buffer_t *buf, void UNUSED *state);
void mktimes(buffer_t *buf, void UNUSED *state);
void new_mail(buffer_t *buf, void *state);

