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

#include <X11/Xlib.h>

#include "config.h"
#include "function.h"

int main(void) {
	char *status;
	char *avgs;
	char *core = NULL;
	char *mem = NULL;
	char *net;
	char *batt = NULL;
	char *datetime = NULL;
	time_t count6 = 0;
	time_t count60 = 0;

	network_init();

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "dwmstatus: cannot open display.\n");
		return 1;
	}

	for (;;sleep(INTERVAL)) {
		if (runevery(&count6, 6)) {
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

		net = network();		

		status = smprintf("%s%s%s%s%s%s%s", avgs, core, mem, net, batt, datetime);
		setstatus(status);
		free(status);
		free(net);
	}

	XCloseDisplay(dpy);
	return 0;
}
