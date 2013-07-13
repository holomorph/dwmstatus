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

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <pulse/pulseaudio.h>

#define UNUSED __attribute__((unused))
#define IO_NEW(io, info, pp) \
  io = calloc(1, sizeof(struct io_t)); \
  io->idx = info->index; \
  io->mute = info->mute; \
  io->name = strdup(info->name ? info->name : ""); \
  io->pp_name = pp; \
  memcpy(&io->volume, &info->volume, sizeof(pa_cvolume)); \
  memcpy(&io->channels, &info->channel_map, sizeof(pa_channel_map)); \
  populate_levels(io);

enum mode {
  MODE_DEVICE = 1 << 0,
  MODE_APP    = 1 << 1,
  MODE_ANY    = (1 << 2) - 1,
};

struct io_t {
  uint32_t idx;
  char *name;
  char *desc;
  const char *pp_name;
  pa_cvolume volume;
  pa_channel_map channels;
  int volume_percent;
  int balance;
  int mute;

  struct ops_t {
    pa_operation *(*mute)(pa_context *, uint32_t, int, pa_context_success_cb_t, void *);
    pa_operation *(*setvol)(pa_context *, uint32_t, const pa_cvolume *, pa_context_success_cb_t, void *);
    pa_operation *(*setdefault)(pa_context *, const char *, pa_context_success_cb_t, void *);
    pa_operation *(*move)(pa_context *, uint32_t, uint32_t, pa_context_success_cb_t, void *);
    pa_operation *(*kill)(pa_context *, uint32_t, pa_context_success_cb_t, void *);
  } op;

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

/* functions */
/* static void io_list_add(struct io_t **list, struct io_t *node); */
/* static void populate_levels(struct io_t *node); */
/* static struct io_t *sink_new(const pa_sink_info *info); */
/* static void sink_add_cb(pa_context UNUSED *c, const pa_sink_info *i, int eol, void *raw); */
/* static void server_info_cb(pa_context UNUSED *c, const pa_server_info *i, void *raw); */
/* static void connect_state_cb(pa_context *cxt, void *raw); */
/* static void pulse_async_wait(struct pulseaudio_t *pulse, pa_operation *op); */
/* static int get_default_sink(struct pulseaudio_t *pulse, struct io_t **list); */
/* static int pulse_init(struct pulseaudio_t *pulse); */
/* static void pulse_deinit(struct pulseaudio_t *pulse); */

/* variable */
/* static struct pulseaudio_t pulse; */

void io_list_add(struct io_t **list, struct io_t *node) {
  struct io_t *head = *list;

  if (head == NULL)
    head = node;
  else {
    head->prev->next = node;
    node->prev = head->prev;
  }

  head->prev = node;
  *list = head;
}

void populate_levels(struct io_t *node) {
  node->volume_percent = (int)(((double)pa_cvolume_avg(&node->volume) * 100)
                               / PA_VOLUME_NORM);
  node->balance = (int)((double)pa_cvolume_get_balance(&node->volume,
                                                       &node->channels) * 100);
}

struct io_t *sink_new(const pa_sink_info *info) {
  struct io_t *sink;

  IO_NEW(sink, info, "sink");
  sink->desc = strdup(info->description);
  sink->op.mute = pa_context_set_sink_mute_by_index;
  sink->op.setvol = pa_context_set_sink_volume_by_index;
  sink->op.setdefault = pa_context_set_default_sink;

  return sink;
}

void sink_add_cb(pa_context UNUSED *c, const pa_sink_info *i, int eol, void *raw) {
  struct cb_data_t *pony = raw;
  if (eol)
    return;
  if (pony->glob && strstr(i->name, pony->glob) == NULL)
    return;
  io_list_add(pony->list, sink_new(i));
}

void server_info_cb(pa_context UNUSED *c, const pa_server_info *i, void *raw) {
  struct pulseaudio_t *pulse = raw;
  pulse->default_sink = strdup(i->default_sink_name);
  pulse->default_source = strdup(i->default_source_name);
}

void connect_state_cb(pa_context *cxt, void *raw) {
  enum pa_context_state *state = raw;
  *state = pa_context_get_state(cxt);
}

void pulse_async_wait(struct pulseaudio_t *pulse, pa_operation *op) {
  while (pa_operation_get_state(op) == PA_OPERATION_RUNNING)
    pa_mainloop_iterate(pulse->mainloop, 1, NULL);
}

int get_default_sink(struct pulseaudio_t *pulse, struct io_t **list) {
  pa_operation *op;
  struct cb_data_t pony = { .list = list };

  op = pa_context_get_sink_info_by_name(pulse->cxt, pulse->default_sink, sink_add_cb, &pony);

  pulse_async_wait(pulse, op);
  pa_operation_unref(op);
  return 0;
}

int pulse_init(struct pulseaudio_t *pulse) {
  pa_operation *op;
  enum pa_context_state state = PA_CONTEXT_CONNECTING;

  pulse->mainloop = pa_mainloop_new();
  pulse->cxt = pa_context_new(pa_mainloop_get_api(pulse->mainloop), "bestpony");
  pulse->default_sink = NULL;
  pulse->default_source = NULL;

  pa_context_set_state_callback(pulse->cxt, connect_state_cb, &state);
  pa_context_connect(pulse->cxt, NULL, PA_CONTEXT_NOFLAGS, NULL);
  while (state != PA_CONTEXT_READY && state != PA_CONTEXT_FAILED)
    pa_mainloop_iterate(pulse->mainloop, 1, NULL);

  if (state != PA_CONTEXT_READY) {
    fprintf(stderr, "failed to connect to pulse daemon: %s\n",
            pa_strerror(pa_context_errno(pulse->cxt)));
    return 1;
  }

  op = pa_context_get_server_info(pulse->cxt, server_info_cb, pulse);
  pulse_async_wait(pulse, op);
  pa_operation_unref(op);
  return 0;
}

void pulse_deinit(struct pulseaudio_t *pulse) {
  pa_context_disconnect(pulse->cxt);
  pa_mainloop_free(pulse->mainloop);
  free(pulse->default_sink);
  free(pulse->default_source);
}

char *ponyprint(void) {
  struct arg_t arg = { 0, NULL, NULL };

  /* initialize connection */
  /* if (pulse_init(&pulse) != 0) */
  /*   return EXIT_FAILURE; */

  get_default_sink(&pulse, &arg.devices);

  if(arg.devices->mute)
    /* printf("VOL: %d%% [MM]\n", arg.devices->volume_percent); */
    return smprintf(VOL_MUTE_STR, arg.devices->volume_percent);
  else
    /* printf("VOL: %d%% [  ]\n", arg.devices->volume_percent); */
    return smprintf(VOL_STR, arg.devices->volume_percent);

  /* shut down */
  /* pulse_deinit(&pulse); */

  /* return EXIT_SUCCESS; */
}

/* vim: set ts=2 sw=2 et: */
