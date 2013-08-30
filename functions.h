/* See LICENSE file for copyright and license details. */

typedef struct {
	char *name;
	char *rx;
	char *tx;
	long rx_bytes;
	long tx_bytes;
} Interface;

typedef struct {
	struct mpd_connection *conn;
	struct mpd_status *status;
} MpdClient;

/* utility functions */
void setstatus(Display *dpy, char *str);
char *smprintf(char *fmt, ...);
int runevery(time_t *ltime, int sec);

/* control abstractions */
int network_init(Interface *iface, char *ifname);
void network_deinit(Interface *iface);
int mpd_init(MpdClient *mpd);
void mpd_deinit(MpdClient *mpd);

/* status elements */
char *loadavg(void);
char *coretemp(void);
char *memory(void);
char *network(Interface *iface, long rx_old, long tx_old);
char *battery(void);
char *mktimes(int tmsleep);
char *music(MpdClient *mpd);
char *new_mail(char *maildir);
