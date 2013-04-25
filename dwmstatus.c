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

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "dwmstatus: cannot open display.\n");
		return 1;
	}

  FILE *f;
  f = fopen(WIFI_DN,"r");
  fscanf(f,"%ld",&rx_old); fclose(f);
  f = fopen(WIFI_UP,"r");
  fscanf(f,"%ld",&tx_old); fclose(f);

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

		if (runevery(&count60, 60)) {
			free(datetime);
			datetime = mktimes();
		}

		net = network();		
    rx_old = rx_new;
    tx_old = tx_new;

		status = smprintf("%s%s%s%s%s%s%s", avgs, core, mem, net, batt, datetime);
		setstatus(status);
		free(status);
		free(net);
	}

	XCloseDisplay(dpy);
	return 0;
}
