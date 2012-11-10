#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <mpd/client.h>
#include <X11/Xlib.h>

#define BATT_LOW 11 // Threshold % battery, below which the display turns urgent
#define CPU_HI   50 // Threshold usage %, above which the display turns urgent
#define INTERVAL 2  // Sleeps for INTERVAL seconds between updates

// Files read for system info:
#define CPU_FILE  "/proc/stat"
#define MEM_FILE  "/proc/meminfo"
#define AUD_FILE  "/home/mvo/.status_info"
#define BATT_POW  "/sys/class/power_supply/BAT1/power_now"
#define BATT_NOW  "/sys/class/power_supply/BAT1/energy_now"
#define BATT_FULL "/sys/class/power_supply/BAT1/energy_full"
#define BATT_STAT "/sys/class/power_supply/BAT1/status"
#define WIFI_DN   "/sys/class/net/wlan0/statistics/rx_bytes"
#define WIFI_UP   "/sys/class/net/wlan0/statistics/tx_bytes"
#define CPU_TEMP0 "/sys/devices/platform/coretemp.0/temp1_input"
// actually temp*_input for *=1,2,3, but I'll just use 1

// Display format strings:
//  Defaults make extensive use of escape characters for colors which require
//  colorstatus patch.  There are also "extended" characters selected to work
//  with terminus2 font for icons.

// Music player daemon
#define MPD_STR       "\x05Î %s "            // mpd playing
#define MPD_P_STR     "\x05Î Paused "        //  "  paused
#define MPD_S_STR     "\x05Î Stopped "       //  "  stopped
#define MPD_NONE_STR  "\x03Î Failed "        //  "  not running

// Volume
#define VOL_STR       "\x04Ô %d%% \x09Ý "    // volume as a percent
#define VOL_MUTE_STR  "\x03Ô × \x09Ý "       // muted

// Core usage, temperature, and memory usage
#define CPU_STR       "\x01Ï %d%%  "         // cpu usage
#define CPU_HI_STR    "\x03Ï %d%%  "         // cpu usage urgent if above CPU_HI
#define CPU_TEMP_STR  "\x01Ç %dC  "          // core temperature (C)
#define MEM_STR       "\x01Þ %dM \x09Ý\x01 " // mem usage including cache/buffer

// Wireless usage (wlan0)
#define WIFI_STR      "\x07Ð %-4s \x06Ñ %-3s \x09Ý\x01 " // wlan0 down/up rates

// Battery status, level, time remaining
#define BAT_STR       "\x08Á %d%% (%02.2f) "   // battery discharging & time left
#define BAT_LOW_STR   "\x03Á %d%% (%02.2f) "   // " urgent if below BAT_LOW
#define BAT_CHRG_STR  "\x08Å Á %d%% (%d:%02.0f) " // " charging
#define BAT_FULL_STR  "\x08À Á %d%% "             // " full

// Date-time
#define DATE_TIME_STR "\x02 Õ %a %d %b %H:%M "    // date-time

//
//
//
static Display *dpy;

void setstatus(char *str) {
  XStoreName(dpy, DefaultRootWindow(dpy), str);
  XSync(dpy, False);
}

int main() {
// Declare all the vars we need
  struct mpd_song * song = NULL;
  char * title = NULL;
  char * artist = NULL;
  int num, hours;
  float timeleft;
  long coretemp;
  long rx_old,tx_old,rx_new,tx_new;
  long jif1,jif2,jif3,jift;
  long lnum1,lnum2,lnum3,lnum4;
  char statnext[30], status[200];
  char rxk[7], txk[7];
  time_t current;
  FILE *infile;
// Evaluate initial jiffies
  infile = fopen(CPU_FILE,"r");
  fscanf(infile,"cpu %ld %ld %ld %ld",&jif1,&jif2,&jif3,&jift);
  fclose(infile);
// Evaluate initial wirelss statistics
  infile = fopen(WIFI_DN,"r");
  fscanf(infile,"%d",&rx_old);fclose(infile);
  infile = fopen(WIFI_UP,"r");
  fscanf(infile,"%d",&tx_old);fclose(infile);
// Setup X display
  if (!(dpy = XOpenDisplay(NULL))) {
    fprintf(stderr, "dwmstatus: could not open display.\n");
    return 1;
  }
  sleep(INTERVAL); // -.-
// MAIN STATUS LOOP STARTS HERE:
  for (;;sleep(INTERVAL)) {
  // reset/clear the status
    status[0]='\0';
  // Music player daemon
  //  the string should be truncated because the status only allocates
  //  so many chars and too long a song title will result in bad behavior.
  //  Also I will probably get rid of the call for artist.
    struct mpd_connection * conn = mpd_connection_new(NULL, 0, 30000);
    mpd_command_list_begin(conn, true);
    mpd_send_status(conn);
    mpd_send_current_song(conn);
    mpd_command_list_end(conn);
  // Get whether mpd is connected
    struct mpd_status* theStatus = mpd_recv_status(conn);
    if (!theStatus)
      sprintf(statnext,MPD_NONE_STR);
    else
      if (mpd_status_get_state(theStatus) == MPD_STATE_PLAY) {
        mpd_response_next(conn);
        song = mpd_recv_song(conn);
        title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
        artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
        sprintf(statnext,MPD_STR,title,artist);
        mpd_song_free(song);
      }
      else if (mpd_status_get_state(theStatus) == MPD_STATE_PAUSE) {
        mpd_response_next(conn);
        song = mpd_recv_song(conn);
        title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
        artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
        sprintf(statnext,MPD_P_STR,title,artist);
        mpd_song_free(song);
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
    infile = fopen(AUD_FILE,"r");
    fscanf(infile,"%d",&num);
    fclose(infile);
    if (num == -1)
    // volume is muted
      sprintf(statnext,VOL_MUTE_STR,num);
    else
    // volume isn't muted
      sprintf(statnext,VOL_STR,num);
    strcat(status,statnext);

  // CPU use:
  //  reads CPU_FILE for the user (U), nice (N), system (S), & idle (I)
  //  stats in that order. Let T = U+N+S+I. Computes usage according to:
  //    usage = (1000*((T-T0) - (I - I0))/(T-T0) + 5)/10
  //  Where appended 0 implies value from previous iteration. See here:
  //  http://colby.id.au/node/39
    infile = fopen(CPU_FILE,"r");
    fscanf(infile,"cpu %ld %ld %ld %ld",&lnum1,&lnum2,&lnum3,&lnum4);
    fclose(infile);
    if (lnum4>jift) // make sure some value has changed from last iter
      num = (int) (1000*((lnum1-jif1)+(lnum2-jif2)+(lnum3-jif3))/((lnum1-jif1)+(lnum2-jif2)+(lnum3-jif3)+(lnum4-jift)) + 5)/10;
    else // if it hasn't, then usage is essentially zero
      num = 0;
  // overwrite old values with new ones
    jif1=lnum1; jif2=lnum2; jif3=lnum3; jift=lnum4;
    if (num > CPU_HI)
    // display cpu usage as urgent
      sprintf(statnext,CPU_HI_STR,num);
    else
    // normal display
      sprintf(statnext,CPU_STR,num);
    strcat(status,statnext);
  // Core temperature
  //  The file CPU_TEMP0 gives temp in degrees C with three appended zeroes
    infile = fopen(CPU_TEMP0,"r");
    fscanf(infile,"%ld\n",&coretemp);
    fclose(infile);
    sprintf(statnext,CPU_TEMP_STR,coretemp/1000);
    strcat(status,statnext);
  // Memory use:
  //  Get the used memory with buffers/cache as in `free -m`
    infile = fopen(MEM_FILE,"r");
    fscanf(infile,"MemTotal: %ld kB\nMemFree: %ld kB\nBuffers: %ld kB\nCached: %ld kB\n",&lnum1,&lnum2,&lnum3,&lnum4);
    fclose(infile);
    sprintf(statnext,MEM_STR,(lnum1-lnum2-lnum3-lnum4)/1024);
    strcat(status,statnext);
  // Wireless network usage
  //  Gets download/upload totals from WIFI_DN & WIFI_UP and computes
  //  the difference between new and old values at each step.
    infile = fopen(WIFI_DN,"r");
    fscanf(infile,"%d",&rx_new);fclose(infile);
    infile = fopen(WIFI_UP,"r");
    fscanf(infile,"%d",&tx_new);fclose(infile);
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
    infile = fopen(BATT_NOW,"r");
    fscanf(infile,"%ld\n",&lnum1);fclose(infile);
    infile = fopen(BATT_FULL,"r");
    fscanf(infile,"%ld\n",&lnum2);fclose(infile);
    infile = fopen(BATT_STAT,"r");
    fscanf(infile,"%s\n",statnext);fclose(infile);
    infile = fopen(BATT_POW,"r");
    fscanf(infile,"%ld\n",&lnum3);fclose(infile);
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
  // Date & Time:
    time(&current);
    strftime(statnext,38,DATE_TIME_STR,localtime(&current));
    strcat(status,statnext);
  // Set root name
    setstatus(status);
  }

  free(status);
  XCloseDisplay(dpy);
  return 0;
}

