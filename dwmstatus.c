/* See LICENSE file for copyright and license details. */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <getopt.h>
#include <X11/Xlib.h>

#include "config.h"
#include "functions.h"
#include "pulse.h"

static Display *dpy;
static long rx_old, tx_old;
static int tmsleep = 0;
static time_t count10 = 0;
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
static char *batt;
static char *date;

void cleanup(void) {
	free(mail);
	free(vol);
	free(avgs);
	free(core);
	free(mem);
	free(net);
	free(batt);
	free(date);
	free(status);
	network_deinit(iface);
	pulse_deinit(&pulse);
	XCloseDisplay(dpy);
}

void sighandler(int signum) {
	switch(signum) {
		case SIGINT:
		case SIGTERM:
			cleanup();
			exit(EXIT_SUCCESS);
	}
}

int main(int argc, char *argv[]) {
	int opt;
	char *ifname = NULL;
	char *maildir = NULL;
	while((opt = getopt(argc, argv, "i:m:")) != -1)
		switch(opt) {
			case 'i':
				ifname = optarg;
				break;
			case 'm':
				maildir = optarg;
				break;
			default:
				return EXIT_FAILURE;
		}

	if(!(iface = malloc(sizeof(Interface)))) {
		fprintf(stderr, "iface malloc failed");
		return EXIT_FAILURE;
	}
	network_init(iface, ifname);
	rx_old = iface->rx_bytes;
	tx_old = iface->tx_bytes;
	if(pulse_init(&pulse) != 0)
		return EXIT_FAILURE;
	if(!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "cannot open display\n");
		return EXIT_FAILURE;
	}

	signal(SIGTERM, sighandler);
	signal(SIGINT, sighandler);

	for(;;sleep(INTERVAL)) {
		if (runevery(&count10, 10)) {
			vol = ponyprint(pulse);
			avgs = loadavg();
			core = coretemp();
			mem = memory();
			batt = battery();
		}
		if(runevery(&count60, tmsleep)) {
			date = mktimes(tmsleep);
			if(runevery(&count180, 180)) {
				mail = new_mail(maildir);
			}
		}

		net = network(iface, rx_old, tx_old);
		rx_old = iface->rx_bytes;
		tx_old = iface->tx_bytes;
		status = smprintf(STATUS, mail, vol, avgs, core, mem, net, batt, date);

		setstatus(dpy, status);
	}

	cleanup();
	return EXIT_SUCCESS;
}
