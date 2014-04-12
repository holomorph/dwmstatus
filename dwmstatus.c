/* See LICENSE file for copyright and license details. */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <X11/Xlib.h>

#include "config.h"

#include "functions.h"
#include "alsa.h"
#include "pulse.h"

static Display *dpy;
static Interface *iface;
static long rx_old, tx_old;
static int pa = 1;
static struct pulseaudio_t pulse;
static char *status;
static char *mail;
static char *addr;
static char *batt;
static char *date;

static struct {
	const char *iface;
	const char *maildir;
	const char *mailbox;
} cfg;

static void setstatus(char *str) {
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}

static void parse_args(int *argc, char **argv[]) {
	static const struct option opts[] = {
		{ "interface", required_argument, 0, 'i' },
		{ "maildir",   required_argument, 0, 'm' },
		{ 0, 0, 0, 0 }
	};

	while (1) {
		int opt = getopt_long(*argc, *argv, "i:m:", opts, NULL);
		if (opt == -1)
			break;

		switch(opt) {
			case 'i':
				cfg.iface = optarg;
				break;
			case 'm':
				cfg.maildir = optarg;
				break;
		}
	}
}

static int dwmstatus_init(void) {
	/* initialize a bunch of things:
	 *
	 * - x display connection
	 * - network info struct
	 * - read initial rx/tx values
	 * - connection to the pulse server
	 * - mailbox location
	 */
	if(!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "cannot open display\n");
		return EXIT_FAILURE;
	}

	if(!(iface = malloc(sizeof(Interface)))) {
		fprintf(stderr, "iface malloc failed");
		return EXIT_FAILURE;
	}
	if(network_init(iface, cfg.iface)) {
		network_deinit(iface);
		return EXIT_FAILURE;
	}
	rx_old = iface->rx_bytes;
	tx_old = iface->tx_bytes;

	if(pulse_init(&pulse) != 0)
		pa = 0;
	cfg.mailbox = get_maildir(cfg.maildir);
	return EXIT_SUCCESS;
}

static int runevery(time_t *ltime, int sec) {
	time_t now = time(NULL);

	if (difftime(now, *ltime) >= sec) {
		*ltime = now;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

static char *volume(void) {
	if(pa)
		return ponyprint(pulse, &pa);
	return alsaprint();
}

static char *render_table(char **table, size_t table_len, const char *sep) {
	size_t i, len = 0, slen = strlen(sep);
	char *ret = NULL, *p;
	unsigned int n = 0, k = 0, first = 0;

	/* find the number n of nonempty elements and total buffer length */
	for (i = 0; i < table_len; ++i)
		if (table[i]) {
			len += strlen(table[i]);
			n++;
			if (n > 1)
				len += slen;
		}

	ret = malloc(++len);
	if (!ret) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	/* print the first element if it exists */
	while (table[first] == NULL)
		first++;
	if (first < len) {
		p = stpcpy(ret, table[first]);
		k++;
	}
	else {
		free(ret);
		return NULL;
	}

	/* print the rest */
	for (i = ++first; (i < --len && k < n); ++i)
		if (table[i]) {
			p = stpcpy(p, sep);
			p = stpcpy(p, table[i]);
			k++;
		}
	return ret;
}

int main(int argc, char *argv[]) {
	time_t count60 = 0;
	time_t count180 = 0;
	int tmsleep = 0;

	cfg.maildir = getenv("MAILDIR");
	parse_args(&argc, &argv);

	if (dwmstatus_init()) {
		fprintf(stderr, "failed to initialize\n");
		return EXIT_FAILURE;
	}

	for(;;sleep(INTERVAL)) {
		if(runevery(&count60, tmsleep)) {
			addr = ipaddr(cfg.iface);
			batt = battery();
			date = mktimes(&tmsleep);
			if(runevery(&count180, 180)) {
				mail = new_mail(cfg.mailbox);
			}
		}

		char *volume_str = volume();
		char *loadavg_str = loadavg();
		char *coretemp_str = coretemp();
		char *memory_str = memory();
		char *network_str = network(iface, rx_old, tx_old);

		rx_old = iface->rx_bytes;
		tx_old = iface->tx_bytes;

		char *table[] = {
			mail,
			volume_str,
			loadavg_str,
			coretemp_str,
			memory_str,
			network_str,
			addr,
			batt,
			date
		};
		const size_t len = sizeof(table) / sizeof(table[0]);
		size_t i;

		status = render_table(table, len, " \x09| ");
		setstatus(status);

		free(volume_str);
		free(loadavg_str);
		free(coretemp_str);
		free(memory_str);
		free(network_str);
	}

	return EXIT_SUCCESS;
}
