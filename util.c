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

void buffer_clear(buffer_t *buf) {
    buf->len = 0;
    if (buf->data)
        buf->data[buf->len] = '\0';
}

void *buffer_new(void) {
    buffer_t *p = malloc(sizeof(buffer_t));
    if(!p) {
        perror("malloc");
        return NULL;
    }
    p->data = NULL;
    p->len = 0;
    return p;
}

size_t buffer_printf(buffer_t *buf, const char *fmt, ...) {
    va_list ap;
    size_t ret;

    va_start(ap, fmt);
    ret = vsnprintf(buf->data, buf->len, fmt, ap);
    va_end(ap);

    if (ret >= buf->len) {
        buf->data = realloc(buf->data, ret + 1);

        va_start(ap, fmt);
        ret = vsnprintf(buf->data, ret + 1, fmt, ap);
        va_end(ap);

        buf->len = ret + 1;
    }
    return ret;
}
