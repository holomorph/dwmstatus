#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>

#include <mpd/client.h>
#include <X11/Xlib.h>

int getloadavg(double loadavg[], int nelem);

static Display *dpy;
static long rx_old, rx_new;
static long tx_old, tx_new;
static int tmsleep = 0;
/* static struct pulseaudio_t pulse; */
static struct mpd_connection *conn;
static char *status;
static char *mail;
static char *mpd;
/* static char *vol; */
static char *avgs;
static char *core;
static char *mem;
static char *net;
static char *batt;
static char *date;

#include "config.h"
#include "function.h"
/* #include "pulse.h" */

static void sighandler(int signum) {
	switch(signum) {
		case SIGINT:
		case SIGTERM:
			free(mail);
			free(mpd);
			/* free(vol); */
			free(avgs);
			free(core);
			free(mem);
			free(net);
			free(batt);
			free(date);
			free(status);
			mpd_connection_free(conn);
			/* pulse_deinit(&pulse); */
			XCloseDisplay(dpy);
			exit(EXIT_SUCCESS);
	}
}

int main(void) {
	char *home = strcat(getenv("HOME"), MAIL_DIR);
	time_t count10 = 0;
	time_t count60 = 0;
	time_t count180 = 0;

	signal(SIGTERM, sighandler);
	signal(SIGINT, sighandler);

	network_init();

/* 	pulse_init(&pulse, "lolpulse"); */
/* 	pulse_connect(&pulse); */

	conn = mpd_connection_new(NULL, 0, 30000);
	if(mpd_connection_get_error(conn)) {
		fprintf(stderr, "failed to connect to mpd: %s\n",
				mpd_connection_get_error_message(conn));
		return 1;
	}

	if(!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "cannot open display\n");
		return 1;
	}

	for(;;sleep(INTERVAL)) {
		if (runevery(&count10, 10)) {
			free(avgs);
			free(core);
			free(mem);
			free(batt);
			avgs = loadavg();
			core = coretemp();
			mem = memory();
			batt = battery();
		}
		if(runevery(&count60, tmsleep)) {
			free(date);
			date = mktimes();
			if(runevery(&count180, 180)) {
				free(mail);
				mail = new_mail(home);
			}
		}

		free(mpd);
		/* free(vol); */
		free(net);
		free(status);
		mpd = print_mpd(conn);
		/* vol = volume(pulse); */
		net = network();
		status = smprintf("%s%s%s%s%s%s%s%s", mail, mpd, avgs, core, mem, net, batt, date);

		setstatus(status);
	}

	mpd_connection_free(conn);
	/* pulse_deinit(&pulse); */
	XCloseDisplay(dpy);
	return 0;
}
