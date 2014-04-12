/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include "util.h"

char *smprintf(const char *fmt, ...) {
	char *buf;
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = vasprintf(&buf, fmt, ap);
	va_end(ap);

	if(ret < 0)
		return NULL;
	return buf;
}
