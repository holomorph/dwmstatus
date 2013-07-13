#define _BSD_SOURCE
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

typedef struct {
	struct mpd_connection *conn;
	struct mpd_status *status;
} MpdClient;

/* static void cleanup(void); */
/* static int runevery(time_t *ltime, int sec); */
/* static void setstatus(char *str); */
/* static void sighandler(int signum); */
/* static char *smprintf(char *fmt, ...); */

static Display *dpy;
static long rx_old, rx_new;
static long tx_old, tx_new;
static int tmsleep = 0;
/* static struct pulseaudio_t pulse; */
static MpdClient *mpd;
static char *status;
static char *mail;
static char *mpdstr;
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

void cleanup(void) {
	free(mail);
	free(mpdstr);
	/* free(vol); */
	free(avgs);
	free(core);
	free(mem);
	free(net);
	free(batt);
	free(date);
	free(status);
	mpd_deinit();
	/* pulse_deinit(&pulse); */
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
	mpd_connect();

	if(!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "cannot open display\n");
		return EXIT_FAILURE;
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

		free(mpdstr);
		/* free(vol); */
		free(net);
		free(status);
		mpdstr = music();
		/* vol = volume(pulse); */
		net = network();
		status = smprintf("%s%s%s%s%s%s%s%s", mail, mpdstr, avgs, core, mem, net, batt, date);

		setstatus(status);
	}

	cleanup();
	return EXIT_SUCCESS;
}
