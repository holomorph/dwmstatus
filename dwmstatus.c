#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <mpd/client.h>
#include <X11/Xlib.h>

#include "dwmstatus.h"

static Display *dpy;

void setstatus(char *str) {
  XStoreName(dpy, DefaultRootWindow(dpy), str);
  XSync(dpy, False);
}

// Volume status
//  AUD_FILE must be present and contain an int between (inclusive) -1 and 100.
//  A script to write to AUD_FILE on volume changes is needed.
void print_volume(char *volume) {
  FILE *f;
  int on, level;
  f = fopen(AUD_FILE,"r");
  fscanf(f,"%d\n%d", &on, &level);
  fclose(f);
  if (on == 0) // volume is muted
    sprintf(volume, VOL_MUTE_STR, level);
  else // volume isn't muted
    sprintf(volume, VOL_STR, level);
}

// Core load averages:
void print_load(char *load) {
  double avgs[3];
  getloadavg(avgs, 3);
  if (avgs[0] > LOAD_HI)
    sprintf(load, CPU_HI_STR, 100*avgs[0], 100*avgs[1], 100*avgs[2]);
  else
    sprintf(load, CPU_STR, 100*avgs[0], 100*avgs[1], 100*avgs[2]);
}

// Core temperature:
//  The file CPU_TEMP0 gives temp in degrees C with three appended zeroes
void print_coretemp(char *coretemp) {
  FILE *f;
  int temp;
  f = fopen(CPU_TEMP0,"r");
  fscanf(f,"%d000\n", &temp);
  fclose(f);
  temp = temp/1000;
  if (temp >= TEMP_HI)
    sprintf(coretemp, TEMP_HI_STR, temp);
  else
    sprintf(coretemp, TEMP_STR, temp);
}

// Memory use:
//  Get the used memory with buffers/cache as in `free -m`
void print_memory(char *mem) {
  FILE *f;
  int total, free, buffers, cached;
  int total_used;
  if(!(f = fopen(MEM_FILE, "r"))) {
    fprintf(stderr, "Cannot open %s for reading.\n",MEM_FILE);
    return;
  }
  fscanf(f, "MemTotal: %d kB\n", &total);
  fscanf(f, "MemFree: %d kB\n",  &free);
  fscanf(f, "Buffers: %d kB\n",  &buffers);
  fscanf(f, "Cached: %d kB\n",   &cached);
  total_used = (total - free - buffers - cached)/1024;
  fclose(f);
  snprintf(mem, 20, MEM_STR, total_used);
}

// Battery status and level
void print_battery(char *battery) {
  FILE *f;
  char state[20];
  int percent;
  float timeleft;
  long now, full, power;
  // scanny
  f = fopen(BATT_NOW,"r");
  fscanf(f,"%ld\n",&now);
  fclose(f);
  f = fopen(BATT_FULL,"r");
  fscanf(f,"%ld\n",&full);
  fclose(f);
  f = fopen(BATT_STAT,"r");
  fscanf(f,"%s\n",state);
  fclose(f);
  f = fopen(BATT_POW,"r");
  fscanf(f,"%ld\n",&power);
  fclose(f);
  // remaining battery percent
  percent = now*100/full;
  if (strncmp(state,"Charging",8) == 0) {
    timeleft = (float) (full-now)/power;   // time until charged
    sprintf(battery,BAT_CHRG_STR,percent,timeleft);
  }
  else if (strncmp(state,"Full",8) == 0) {
    sprintf(battery,BAT_FULL_STR,percent);
  }
  else {
    timeleft = (float) now/power; // time until discharged
    if (percent < BATT_LOW) // battery urgent if below BATT_LOW
      sprintf(battery,BAT_LOW_STR,percent,timeleft);
    else // battery normal discharging
      sprintf(battery,BAT_STR,percent,timeleft);
  }
}

// Date & time:
void print_datetime(char *datetime) {
  time_t current;
  time(&current);
  strftime(datetime, 38, DATE_TIME_STR, localtime(&current));
}

// MPD Now playing
void print_mpd(char *mpd) {
  int mute;
  struct mpd_connection *conn = mpd_connection_new(NULL, 0, 30000);
  mpd_command_list_begin(conn, true);
  mpd_send_status(conn);
  mpd_send_current_song(conn);
  mpd_command_list_end(conn);
  struct mpd_status *theStatus = mpd_recv_status(conn); // connected?
  if (!theStatus)
    sprintf(mpd,MPD_NONE_STR);
  else
    if (mpd_status_get_state(theStatus) == MPD_STATE_PLAY) {
      mpd_response_next(conn);
      struct mpd_song *song = mpd_recv_song(conn);
      const char *title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
      if (!title) title = mpd_song_get_tag(song, MPD_TAG_NAME, 0);
      sprintf(mpd,MPD_STR,title);
      mpd_song_free(song);
    }
    else if (mpd_status_get_state(theStatus) == MPD_STATE_PAUSE) {
      sprintf(mpd,MPD_P_STR);
    }
    else if (mpd_status_get_state(theStatus) == MPD_STATE_STOP) {
      sprintf(mpd,MPD_S_STR);
    }
  mpd_response_finish(conn);
  mpd_connection_free(conn);
}

// Wireless network usage
//  Gets download/upload totals from WIFI_DN & WIFI_UP and computes
//  the difference between new and old values at each step.
long rx_old,tx_old,rx_new,tx_new;
void print_wifi(char *wifi) {
  FILE *f;
  char rxk[7], txk[7];
  f = fopen(WIFI_DN,"r");
  fscanf(f,"%d",&rx_new);
  fclose(f);
  f = fopen(WIFI_UP,"r");
  fscanf(f,"%d",&tx_new);
  fclose(f);
  sprintf(rxk,"%dK",(rx_new-rx_old)/1024/INTERVAL);
  sprintf(txk,"%dK",(tx_new-tx_old)/1024/INTERVAL);
  sprintf(wifi,WIFI_STR,rxk,txk);
}

int main() {
  // Declare all the vars we need
  char mpd[50];
  char volume[15];
  char load[22];
  char mem[20];
  char coretemp[11];
  char wifi[30];
  char battery[37];
  char datetime[24];

  char statnext[30], status[200];

  // Evaluate initial wireless statistics
  FILE *f;
  f = fopen(WIFI_DN,"r");
  fscanf(f,"%d",&rx_old); fclose(f);
  f = fopen(WIFI_UP,"r");
  fscanf(f,"%d",&tx_old); fclose(f);

  // Setup X display
  if (!(dpy = XOpenDisplay(NULL))) {
    fprintf(stderr, "dwmstatus: could not open display.\n");
    return 1;
  }

  // MAIN STATUS LOOP STARTS HERE:
  for (;;sleep(INTERVAL)) {
    // reset/clear the status
    status[0]='\0';

    // Music player daemon
    print_mpd(mpd);
    strcat(status,mpd);

    // Audio volume:
    print_volume(volume);
    strcat(status,volume);

    // Core load averages
    print_load(load);
    strcat(status,load);

    // Core temperature
    print_coretemp(coretemp);
    strcat(status,coretemp);

    // Memory use:
    print_memory(mem);
    strcat(status,mem);

    // Wireless network usage
    print_wifi(wifi);
    strcat(status,wifi);
    // overwrite old values with new ones
    rx_old = rx_new;
    tx_old = tx_new;

    // battery!!!!!!
    print_battery(battery);
    strcat(status,battery);

    // date time!!
    print_datetime(datetime);
    strcat(status,datetime);

    // Set root name
    setstatus(status);
  }

  free(status);
  XCloseDisplay(dpy);
  return 0;
}
