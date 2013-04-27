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

#include <mpd/client.h>
#include <X11/Xlib.h>

#include "config.h"
#include "function.h"
#include "pulse.h"

static struct pulseaudio_t pulse;

int main(void) {
	char *status;
	char *mpd;
	char *vol;
	char *avgs;
	char *core = NULL;
	char *mem = NULL;
	char *net;
	char *batt = NULL;
	char *datetime = NULL;
	time_t count10 = 0;
	time_t count60 = 0;

	network_init();

	pulse_init(&pulse, "lolpulse");
	pulse_connect(&pulse);

	struct mpd_connection *conn = mpd_connection_new(NULL, 0, 30000);

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "dwmstatus: cannot open display.\n");
		return 1;
	}

	for (;;sleep(INTERVAL)) {
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

		if (runevery(&count60, tmsleep)) {
			free(datetime);
			datetime = mktimes();
		}

		mpd = print_mpd(conn);
		vol = volume(pulse);
		net = network();		
		status = smprintf("%s%s%s%s%s%s%s%s", mpd, vol, avgs, core, mem, net, batt, datetime);

		setstatus(status);

		free(status);
		free(net);
		free(vol);
		free(mpd);
	}

	mpd_connection_free(conn);
	pulse_deinit(&pulse);
	XCloseDisplay(dpy);
	return 0;
}
