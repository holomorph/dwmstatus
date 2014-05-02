/* See LICENSE file for copyright and license details. */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <err.h>
#include <getopt.h>
#include <X11/Xlib.h>

#include "config.h"

#include "functions.h"
#include "alsa.h"
#include "pulse.h"

static Display *dpy;
static int pa = 1;
static struct pulseaudio_t pulse;

static struct {
	char *iface;
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

static void *ifname_init(void) {
	return cfg.iface;
}

static void *maildir_init(void) {
	if(!cfg.maildir || cfg.maildir[0] == '\0')
		return NULL;
	return smprintf("%s/%s/new", cfg.maildir, MAIL_BOX);
}

static void *network_init(void) {
	const char *ifname = cfg.iface;
	Interface *p = malloc(sizeof(Interface));
	if(!p)
		err(errno, "iface malloc");
	p->name = ifname;
	p->rx = smprintf("/sys/class/net/%s/statistics/rx_bytes", ifname);
	p->tx = smprintf("/sys/class/net/%s/statistics/tx_bytes", ifname);
	FILE *f;

	if(!(f = fopen(p->rx, "r"))) {
		network_deinit(p);
		err(errno, "%s", ifname);
	}
	fscanf(f,"%ld", &p->rx_bytes);
	fclose(f);
	p->rx_old = p->rx_bytes;
	if(!(f = fopen(p->tx, "r"))) {
		network_deinit(p);
		err(errno, "%s", ifname);
	}
	fscanf(f,"%ld", &p->tx_bytes);
	fclose(f);
	p->tx_old = p->tx_bytes;
	return p;
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

	if(pulse_init(&pulse) != 0)
		pa = 0;
	cfg.mailbox = maildir_init();
	return EXIT_SUCCESS;
}

static void volume(buffer_t *buf, void UNUSED *state) {
	if(pa)
		ponyprint(buf, pulse, &pa);
	else
		alsaprint(buf);
}

static void render_table(struct item *table, size_t table_len, buffer_t *status, const char *sep) {
	size_t i, len = 0, slen = strlen(sep);
	char *p;
	unsigned int first;

	for (first = 0; first < table_len && table[first].buffer->data == NULL; ++first);
	if (first == table_len) {
		buffer_clear(status);
		return;
	}
	len = strlen(table[first].buffer->data);
	for (i = first + 1; i < table_len; ++i) {
		if (table[i].buffer->data) {
			len += strlen(table[i].buffer->data) + slen;
		}
	}
	if (len >= status->len) {
		status->data = realloc(status->data, (size_t)len + 1);
		status->len = len + 1;
	}

	p = stpcpy(status->data, table[first].buffer->data);
	for (i = first + 1; i < table_len; ++i)
		if (table[i].buffer->data) {
			p = stpcpy(p, sep);
			p = stpcpy(p, table[i].buffer->data);
		}
	return;
}

int main(int argc, char *argv[]) {
	cfg.maildir = getenv("MAILDIR");
	parse_args(&argc, &argv);

	/* init */
	if (dwmstatus_init()) {
		fprintf(stderr, "failed to initialize\n");
		return EXIT_FAILURE;
	}

	struct item mail = { .init = maildir_init, .update = new_mail };
	struct item vol  = { .init = NULL, .update = volume };
	struct item load = { .init = NULL, .update = loadavg };
	struct item temp = { .init = NULL, .update = coretemp };
	struct item mem  = { .init = NULL, .update = memory };
	struct item net  = { .init = network_init, .update = network };
	struct item addr = { .init = ifname_init, .update = ipaddr };
	struct item batt = { .init = NULL, .update = battery };
	struct item date = { .init = NULL, .update = mktimes };
	struct item table[] = {
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
	size_t table_len = sizeof(table) / sizeof(struct item);

	for (unsigned int i = 0; i < table_len; i++) {
		table[i].buffer = buffer_new();
		if (table[i].init)
			table[i].state = table[i].init();
	}
	buffer_t *status = buffer_new();

	for(;;sleep(INTERVAL)) {
		for (unsigned int i = 0; i < table_len; i++)
			table[i].update(table[i].buffer, table[i].state);

		render_table(table, table_len, status, " \x09| ");

		setstatus(status->data);
	}

	return EXIT_SUCCESS;
}
