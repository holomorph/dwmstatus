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

#include <X11/Xlib.h>

#include "config.h"
#include "function.h"

int main(void) {
	char *status;
	char *avgs;
	char *datetime = NULL;
	time_t count6 = 0;
	time_t count60 = 0;

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "dwmstatus: cannot open display.\n");
		return 1;
	}

	for (;;sleep(INTERVAL)) {
		if (runevery(&count6, 6)) {
			free(avgs);
			avgs = loadavg();
		}

		if (runevery(&count60, 60)) {
			free(datetime);
			datetime = mktimes();
		}

		status = smprintf("%s%s", avgs, datetime);
		setstatus(status);
		free(status);
	}

	XCloseDisplay(dpy);
	return 0;
}

