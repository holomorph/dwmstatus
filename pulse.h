#include <err.h>
#include <stdbool.h>

#include <pulse/pulseaudio.h>

#define UNUSED __attribute__((unused))

enum connectstate {
	STATE_CONNECTING = 0,
	STATE_CONNECTED,
	STATE_ERROR
};

struct sink_t {
	uint32_t idx;
	const char *name;
	const char *desc;
	pa_cvolume volume;
	int volume_percent;
	bool mute;

	struct sink_t *next_sink;
};

struct pulseaudio_t {
	pa_context *cxt;
	pa_mainloop *mainloop;
	pa_mainloop_api *mainloop_api;
	enum connectstate state;

	struct sink_t *sinks;
};

static int pulse_async_wait(struct pulseaudio_t *pulse, pa_operation *op) {
	int r;

	while (pa_operation_get_state(op) == PA_OPERATION_RUNNING)
		pa_mainloop_iterate(pulse->mainloop, 1, &r);

	return r;
}

static struct sink_t *pulse_sink_new(const pa_sink_info *info) {
	struct sink_t *sink = calloc(1, sizeof(struct sink_t));

	sink->idx = info->index;
	sink->name = info->name;
	sink->desc = info->description;
	sink->volume.channels = info->volume.channels;

	memcpy(&sink->volume, &info->volume, sizeof(pa_cvolume));
	sink->volume_percent = (int)(((double)pa_cvolume_avg(&sink->volume) * 100.)
															 / PA_VOLUME_NORM);
	sink->mute = info->mute;

	return sink;
}

static void server_info_cb(pa_context UNUSED *c, const pa_server_info *i, void *raw) {
	const char **sink_name = (const char **)raw;
	*sink_name = i->default_sink_name;
}

static void sink_add_cb(pa_context UNUSED *c, const pa_sink_info *i, int eol,
												void *raw) {
	struct pulseaudio_t *pulse = raw;
	struct sink_t *s, *sink;

	if (eol)
		return;

	sink = pulse_sink_new(i);

	if (pulse->sinks == NULL)
		pulse->sinks = sink;
	else {
		s = pulse->sinks;
		s->next_sink = sink;
		pulse->sinks = sink;
	}
}

static void state_cb(pa_context UNUSED *c, void *raw) {
	struct pulseaudio_t *pulse = raw;

	switch (pa_context_get_state(pulse->cxt)) {
		case PA_CONTEXT_READY:
			pulse->state = STATE_CONNECTED;
			break;
		case PA_CONTEXT_FAILED:
			pulse->state = STATE_ERROR;
			break;
		case PA_CONTEXT_UNCONNECTED:
		case PA_CONTEXT_AUTHORIZING:
		case PA_CONTEXT_SETTING_NAME:
		case PA_CONTEXT_CONNECTING:
		case PA_CONTEXT_TERMINATED:
			break;
	}
}

static void get_sink_by_name(struct pulseaudio_t *pulse, const char *name) {
	pa_operation *op = pa_context_get_sink_info_by_name(pulse->cxt, name,
																											sink_add_cb, pulse);
	pulse_async_wait(pulse, op);
	pa_operation_unref(op);
}

static void get_default_sink(struct pulseaudio_t *pulse) {
	const char *sink_name;
	pa_operation *op = pa_context_get_server_info(pulse->cxt, server_info_cb, &sink_name);
	pulse_async_wait(pulse, op);
	pa_operation_unref(op);

	get_sink_by_name(pulse, sink_name);
}

static void pulse_deinit(struct pulseaudio_t *pulse) {
	pa_context_disconnect(pulse->cxt);
	pa_mainloop_free(pulse->mainloop);
	free(pulse->sinks);
}

static void pulse_init(struct pulseaudio_t *pulse, const char *clientname) {
	/* allocate */
	pulse->mainloop = pa_mainloop_new();
	pulse->mainloop_api = pa_mainloop_get_api(pulse->mainloop);
	pulse->cxt = pa_context_new(pulse->mainloop_api, clientname);
	pulse->state = STATE_CONNECTING;
	pulse->sinks = NULL;

	/* set state callback for connection */
	pa_context_set_state_callback(pulse->cxt, state_cb, pulse);
}

static int pulse_connect(struct pulseaudio_t *pulse) {
	int r;

	pa_context_connect(pulse->cxt, NULL, PA_CONTEXT_NOFLAGS, NULL);
	while (pulse->state == STATE_CONNECTING)
		pa_mainloop_iterate(pulse->mainloop, 1, &r);

	if (pulse->state == STATE_ERROR) {
		r = pa_context_errno(pulse->cxt);
		fprintf(stderr, "failed to connect to pulse daemon: %s\n",
						pa_strerror(r));
		return 1;
	}

	return 0;
}

char *volume(struct pulseaudio_t pulse) {
	get_default_sink(&pulse);
	if (pulse.sinks->mute)
		return smprintf(VOL_MUTE_STR, pulse.sinks->volume_percent);
	else
		return smprintf(VOL_STR, pulse.sinks->volume_percent);
}
