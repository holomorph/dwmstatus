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
static Interface *iface;
static long rx_old, tx_old;
static int pa = 1;
static struct pulseaudio_t pulse;
static char *status;
static char *mail;
static char *addr;
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
	free(addr);
	free(date);
	free(status);
	network_deinit(iface);
	pulse_deinit(&pulse);
	XCloseDisplay(dpy);
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

static char *volume(void) {
	if(pa)
		return ponyprint(pulse, &pa);
	return alsaprint();
}

int main(int argc, char *argv[]) {
	time_t count60 = 0;
	time_t count180 = 0;
	int tmsleep = 0;

	parse_args(&argc, &argv);

	if (dwmstatus_init()) {
		fprintf(stderr, "failed to initialize\n");
		return EXIT_FAILURE;
	}

	for(;;sleep(INTERVAL)) {

		if(runevery(&count60, tmsleep)) {
			addr = ipaddr(cfg.iface);
			date = mktimes(&tmsleep);
			if(runevery(&count180, 180)) {
				mail = new_mail(cfg.maildir);
			}
		}

		status = smprintf(STATUS, mail, volume(), loadavg(), coretemp(), memory(), network(iface, rx_old, tx_old), addr, battery(), date);
		rx_old = iface->rx_bytes;
		tx_old = iface->tx_bytes;

		setstatus(status);
	}

	cleanup();
	return EXIT_SUCCESS;
}
