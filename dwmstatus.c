/* See LICENSE file for copyright and license details. */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <getopt.h>
#include <X11/Xlib.h>

#include "config.h"

#include "functions.h"
#include "alsa.h"
#include "pulse.h"

static Display *dpy;
static long rx_old, tx_old;
static int tmsleep = 0;
static int pa = 1;
static time_t count60 = 0;
static time_t count180 = 0;
static struct pulseaudio_t pulse;
static Interface *iface;
static char *status;
static char *mail;
static char *vol;
static char *avgs;
static char *core;
static char *mem;
static char *net;
static char *addr;
static char *batt;
static char *date;

static struct {
	const char *iface;
	const char *maildir;
} cfg;

static void setstatus(char *str) {
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}

static void cleanup(void) {
	free(mail);
	free(vol);
	free(avgs);
	free(core);
	free(mem);
	free(net);
	free(addr);
	free(batt);
	free(date);
	free(status);
	network_deinit(iface);
	pulse_deinit(&pulse);
	XCloseDisplay(dpy);
}

static void parse_args(int *argc, char **argv[]) {
	while (1) {
		int opt = getopt(*argc, *argv, "i:m:");
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

	return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
	parse_args(&argc, &argv);

	if (dwmstatus_init()) {
		fprintf(stderr, "failed to initialize\n");
		return EXIT_FAILURE;
	}

	for(;;sleep(INTERVAL)) {
		if(pa)
			vol = ponyprint(pulse);
		else
			vol = alsaprint();
		avgs = loadavg();
		core = coretemp();
		mem = memory();
		batt = battery();

		if(runevery(&count60, tmsleep)) {
			addr = ipaddr(cfg.iface);
			date = mktimes(tmsleep);
			if(runevery(&count180, 180)) {
				mail = new_mail(cfg.maildir);
			}
		}

		net = network(iface, rx_old, tx_old);
		rx_old = iface->rx_bytes;
		tx_old = iface->tx_bytes;
		status = smprintf(STATUS, mail, vol, avgs, core, mem, net, addr, batt, date);

		setstatus(status);
	}

	cleanup();
	return EXIT_SUCCESS;
}
