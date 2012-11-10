#ifndef __DWMSTATUS_H_INCLUDED__
#define __DWMSTATUS_H_INCLUDED__

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
#define MPD_STR      "\x05Î %s "      // mpd playing
#define MPD_P_STR    "\x05Î Paused "  //  "  paused
#define MPD_S_STR    "\x05Î Stopped " //  "  stopped
#define MPD_NONE_STR "\x03Î Failed "  //  "  not running

// Volume
#define VOL_STR      "\x04Ô %d%% \x09Ý " // volume as a percent
#define VOL_MUTE_STR "\x03Ô × \x09Ý "    // muted

// Core usage, temperature, and memory usage
#define CPU_STR      "\x01Ï %d%%  "         // cpu usage
#define CPU_HI_STR   "\x03Ï %d%%  "         // cpu usage urgent if above CPU_HI
#define CPU_TEMP_STR "\x01Ç %dC  "          // core temperature (C)
#define MEM_STR      "\x01Þ %dM \x09Ý\x01 " // mem usage including cache/buffer

// Wireless usage (wlan0)
#define WIFI_STR     "\x07Ð %-4s \x06Ñ %-3s \x09Ý\x01 " // wlan0 down/up rates

// Battery status, level, time remaining
#define BAT_STR      "\x08Á %d%% (%02.2f) " // battery discharging & time left
#define BAT_LOW_STR  "\x03Á %d%% (%02.2f) " // " urgent if below BAT_LOW
#define BAT_CHRG_STR "\x08Å Á %d%% (%d:%02.0f) " // " charging
#define BAT_FULL_STR "\x08À Á %d%% "             // " full

// Date-time
#define DATE_TIME_STR "\x02 Õ %a %d %b %H:%M "    // date-time

#endif
