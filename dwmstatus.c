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

// Date & time:
void print_datetime(char *datetime) {
  time_t current;
  time(&current);
  strftime(datetime, 38, DATE_TIME_STR, localtime(&current));
}

int main() {
  // Declare all the vars we need
  char load[22];
  char mem[20];
  char coretemp[11];
  char datetime[24];

  struct mpd_song * song = NULL;
  char * title = NULL;
  char * artist = NULL;
  int num, hours;
  int sound;
  float timeleft;

  long rx_old,tx_old,rx_new,tx_new;
  long lnum1,lnum2,lnum3,lnum4;
  char statnext[30], status[200];
  char rxk[7], txk[7];

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

  //sleep(INTERVAL); // -.-

  // MAIN STATUS LOOP STARTS HERE:
  for (;;sleep(INTERVAL)) {
  // reset/clear the status
    status[0]='\0';
  // Music player daemon
  //  the string should be truncated because the status only allocates
  //  so many chars and too long a song title will result in bad behavior.
    struct mpd_connection * conn = mpd_connection_new(NULL, 0, 30000);
    mpd_command_list_begin(conn, true);
    mpd_send_status(conn);
    mpd_send_current_song(conn);
    mpd_command_list_end(conn);
    struct mpd_status* theStatus = mpd_recv_status(conn); // connected?
    if (!theStatus)
      sprintf(statnext,MPD_NONE_STR);
    else
      if (mpd_status_get_state(theStatus) == MPD_STATE_PLAY) {
        mpd_response_next(conn);
        song = mpd_recv_song(conn);
        title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
        sprintf(statnext,MPD_STR,title);
        mpd_song_free(song);
      }
      else if (mpd_status_get_state(theStatus) == MPD_STATE_PAUSE) {
        sprintf(statnext,MPD_P_STR);
      }
      else if (mpd_status_get_state(theStatus) == MPD_STATE_STOP) {
        sprintf(statnext,MPD_S_STR);
      }
    mpd_response_finish(conn);
    mpd_connection_free(conn);
    strcat(status,statnext);

  // Audio volume:
  //  AUD_FILE must be present and contain an int between (inclusive) -1
  //  and 100. A script to write to AUD_FILE on volume changes is needed.
    f = fopen(AUD_FILE,"r");
    fscanf(f,"%d\n%d", &sound, &num);
    fclose(f);
    if (sound == 0) // volume is muted
      sprintf(statnext, VOL_MUTE_STR, num);
    else                // volume isn't muted
      sprintf(statnext, VOL_STR, num);
    strcat(status,statnext);

    // Core load averages
    //
    print_load(load);
    strcat(status, load);

    // Core temperature
    //  The file CPU_TEMP0 gives temp in degrees C with three appended zeroes
    print_coretemp(coretemp);
    strcat(status, coretemp);

    // Memory use:
    //  Get the used memory with buffers/cache as in `free -m`
    print_memory(mem);
    strcat(status, mem);

  // Wireless network usage
  //  Gets download/upload totals from WIFI_DN & WIFI_UP and computes
  //  the difference between new and old values at each step.
    f = fopen(WIFI_DN,"r");
    fscanf(f,"%d",&rx_new);fclose(f);
    f = fopen(WIFI_UP,"r");
    fscanf(f,"%d",&tx_new);fclose(f);
    sprintf(rxk,"%dK",(rx_new-rx_old)/1024);
    sprintf(txk,"%dK",(tx_new-tx_old)/1024);
    sprintf(statnext,WIFI_STR,rxk,txk);
    strcat(status,statnext);
  // overwrite old values with new ones
    rx_old = rx_new;
    tx_old = tx_new;

  // Power / Battery:
  //  reads files which give energy in micro Watt hours. The power
  //  is given in micro Watts so a little arithmetic is needed to
  //  get the time remaining in hours:minutes.
    f = fopen(BATT_NOW,"r");
    fscanf(f,"%ld\n",&lnum1);fclose(f);
    f = fopen(BATT_FULL,"r");
    fscanf(f,"%ld\n",&lnum2);fclose(f);
    f = fopen(BATT_STAT,"r");
    fscanf(f,"%s\n",statnext);fclose(f);
    f = fopen(BATT_POW,"r");
    fscanf(f,"%ld\n",&lnum3);fclose(f);
  // remaining battery percent
    num = lnum1*100/lnum2;
    if (strncmp(statnext,"Charging",8) == 0) {
      timeleft = (float) (lnum2-lnum1)/lnum3;   // time until charged
      hours = (int) timeleft;                   // hours
      timeleft = (float) (timeleft - hours)*60; // mins
      sprintf(statnext,BAT_CHRG_STR,num,hours,timeleft);
    }
    else if (strncmp(statnext,"Full",8) == 0) {
      sprintf(statnext,BAT_FULL_STR,num);
    }
    else {
      timeleft = (float) lnum1/lnum3;           // time until discharged
      if (num < BATT_LOW)
      // battery urgent if below BATT_LOW
        sprintf(statnext,BAT_LOW_STR,num,timeleft);
      else
      // battery normal discharging
        sprintf(statnext,BAT_STR,num,timeleft);
    }
    strcat(status,statnext);

    // date time!!
    print_datetime(datetime); strcat(status,datetime);

    // Set root name
    setstatus(status);
  }

  free(status);
  XCloseDisplay(dpy);
  return 0;
}

