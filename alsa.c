#include <stdio.h>
#include <alloca.h>

#include "alsa.h"
#include "config.h"
#include "util.h"

static snd_mixer_t* alsa_init(const char *card) {
  snd_mixer_t *handle;
  snd_mixer_open(&handle, 0);
  snd_mixer_attach(handle, card);
  snd_mixer_selem_register(handle, NULL, NULL);
  snd_mixer_load(handle);
  return handle;
}

static void alsa_deinit(snd_mixer_t *handle) {
  snd_mixer_close(handle);
}

static snd_mixer_elem_t* alsa_mixer(snd_mixer_t *handle, const char *mixer) {
  snd_mixer_selem_id_t *sid;
  snd_mixer_selem_id_alloca(&sid);
  snd_mixer_selem_id_set_index(sid, 0);
  snd_mixer_selem_id_set_name(sid, mixer);
  return snd_mixer_find_selem(handle, sid);
}

void alsaprint(buffer_t *buf) {
  snd_mixer_t *alsa;
  snd_mixer_elem_t *mixer;
  int mute;
  long vol;

  if(!(alsa = alsa_init("default"))) {
    buffer_clear(buf);
    return;
  }
  if(!(mixer = alsa_mixer(alsa, "Master"))) {
    buffer_clear(buf);
    return;
  }
  snd_mixer_selem_get_playback_switch(mixer, SND_MIXER_SCHN_MONO, &mute);
  snd_mixer_selem_get_playback_volume(mixer, SND_MIXER_SCHN_MONO, &vol);
  vol = vol - 64;
  fflush(stdout);
  snd_mixer_handle_events(alsa);
  alsa_deinit(alsa);
  buffer_printf(buf, mute ? VOL_STR : VOL_MUTE_STR, (int)vol, "dB");
}

/* vim: set ts=2 sw=2 et: */
