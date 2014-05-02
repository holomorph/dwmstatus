/* Copyright (c) 2012 Dave Reisner
 *
 * ponymix.c
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/* modified from tag v1.0
 * https://github.com/falconindy/ponymix
 * /tree/d5eeab41f026d97b25efe00e3df9d7ee75e8d04a */

#include <pulse/pulseaudio.h>

#include "util.h"

#define IO_NEW(io, info, pp) \
  io = calloc(1, sizeof(struct io_t)); \
  io->mute = info->mute; \
  io->name = strdup(info->name ? info->name : ""); \
  io->pp_name = pp; \
  memcpy(&io->volume, &info->volume, sizeof(pa_cvolume)); \
  populate_levels(io);

struct io_t {
  char *name;
  char *desc;
  const char *pp_name;
  pa_cvolume volume;
  int volume_percent;
  int mute;

  struct io_t *next;
  struct io_t *prev;
};

struct cb_data_t {
  struct io_t **list;
  const char *glob;
};

struct pulseaudio_t {
  pa_context *cxt;
  pa_mainloop *mainloop;

  char *default_sink;
  char *default_source;
};

struct arg_t {
  long value;
  struct io_t *devices;
  struct io_t *target;
};

int get_default_sink(struct pulseaudio_t *pulse, struct io_t **list);
int pulse_init(struct pulseaudio_t *pulse);
void pulse_deinit(struct pulseaudio_t *pulse);
void ponyprint(buffer_t *buf, struct pulseaudio_t pulse, int *pa_running);

/* vim: set ts=2 sw=2 et: */
