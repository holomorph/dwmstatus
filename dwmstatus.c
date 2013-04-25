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

int
main(void)
{
	char *status;
	char *avgs;
	char *datetime;

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "dwmstatus: cannot open display.\n");
		return 1;
	}

	for (;;sleep(INTERVAL)) {
		avgs = loadavg();
		datetime = mktimes();

		status = smprintf("%s%s", avgs, datetime);
		setstatus(status);
		free(avgs);
		free(status);
	}

	XCloseDisplay(dpy);

	return 0;
}

