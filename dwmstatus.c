#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <mpd/client.h>
#include <X11/Xlib.h>

#include "config.h"
#include "function.h"

int main() {
  char mpd[50];
  char volume[15];
  char load[22];
  char mem[20];
  char coretemp[11];
  char wifi[30];
  char battery[37];
  char datetime[24];

  char status[200];

  // Evaluate initial wireless statistics
  FILE *f;
  f = fopen(WIFI_DN,"r");
  fscanf(f,"%ld",&rx_old); fclose(f);
  f = fopen(WIFI_UP,"r");
  fscanf(f,"%ld",&tx_old); fclose(f);

  if (!(dpy = XOpenDisplay(NULL))) {
    fprintf(stderr, "dwmstatus: could not open display.\n");
    return 1;
  }

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
