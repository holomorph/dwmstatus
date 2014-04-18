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

static void *maildir_init(void) {
	if(!cfg.maildir || cfg.maildir[0] == '\0')
		return NULL;
	return smprintf("%s/%s/new", cfg.maildir, MAIL_BOX);
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

	iface = network_init(cfg.iface);
	rx_old = iface->rx_bytes;
	tx_old = iface->tx_bytes;

	if(pulse_init(&pulse) != 0)
		pa = 0;
	cfg.mailbox = maildir_init();
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

static void volume(buffer_t *buf) {
	if(pa)
		ponyprint(buf, pulse, &pa);
	else
		alsaprint(buf);
}

static void render_table(buffer_t **table, size_t table_len, buffer_t *status, const char *sep) {
	size_t i, len = 0, slen = strlen(sep);
	char *p;
	unsigned int first;

	for (first = 0; first < table_len && table[first]->data == NULL; ++first);
	if (first == table_len) {
		buffer_clear(status);
		return;
	}
	len = strlen(table[first]->data);
	for (i = first + 1; i < table_len; ++i) {
		if (table[i]->data) {
			len += strlen(table[i]->data) + slen;
		}
	}
	if (len >= status->len) {
		status->data = realloc(status->data, (size_t)len + 1);
		status->len = len + 1;
	}

	p = stpcpy(status->data, table[first]->data);
	for (i = first + 1; i < table_len; ++i)
		if (table[i]->data) {
			p = stpcpy(p, sep);
			p = stpcpy(p, table[i]->data);
		}
	return;
}

int main(int argc, char *argv[]) {
	time_t count60 = 0;
	time_t count180 = 0;
	int tmsleep = 0;

	cfg.maildir = getenv("MAILDIR");
	parse_args(&argc, &argv);

	/* init */
	if (dwmstatus_init()) {
		fprintf(stderr, "failed to initialize\n");
		return EXIT_FAILURE;
	}

	buffer_t *status = buffer_new();
	buffer_t *mail = buffer_new();
	buffer_t *vol  = buffer_new();
	buffer_t *load = buffer_new();
	buffer_t *temp = buffer_new();
	buffer_t *mem  = buffer_new();
	buffer_t *net  = buffer_new();
	buffer_t *addr = buffer_new();
	buffer_t *batt = buffer_new();
	buffer_t *date = buffer_new();

	for(;;sleep(INTERVAL)) {
		if(runevery(&count60, tmsleep)) {
			ipaddr(addr, cfg.iface);
			mktimes(date, &tmsleep);
			if(runevery(&count180, 180)) {
				new_mail(mail, cfg.mailbox);
			}
		}
		volume(vol);
		loadavg(load);
		coretemp(temp);
		memory(mem);
		network(net, iface, rx_old, tx_old);
		battery(batt);

		buffer_t *table[] = {
			mail,
			vol,
			load,
			temp,
			mem,
			net,
			addr,
			batt,
			date
		};
		size_t table_len = sizeof(table) / sizeof(buffer_t *);

		render_table(table, table_len, status, " \x09| ");

		rx_old = iface->rx_bytes;
		tx_old = iface->tx_bytes;

		setstatus(status->data);
	}

	return EXIT_SUCCESS;
}
