#ifndef __DWMSTATUS_H_INCLUDED__
#define __DWMSTATUS_H_INCLUDED__

#define BATT_LOW 11 // Upper threshold for urgent battery %
#define LOAD_HI  1  // Lower threshold for urgent cpu load
#define TEMP_HI  75 // Lower threshold for urgent core temp
#define INTERVAL 2  // Sleeps for INTERVAL seconds each iter

// Files read for system info:
#define MEM_FILE  "/proc/meminfo"
#define AUD_FILE  "/run/user/1000/pulse/volume_info"
#define BATT_POW  "/sys/class/power_supply/BAT1/power_now"
#define BATT_NOW  "/sys/class/power_supply/BAT1/energy_now"
#define BATT_FULL "/sys/class/power_supply/BAT1/energy_full"
#define BATT_STAT "/sys/class/power_supply/BAT1/status"
#define WIFI_DN   "/sys/class/net/wlp2s0/statistics/rx_bytes"
#define WIFI_UP   "/sys/class/net/wlp2s0/statistics/tx_bytes"
#define CPU_TEMP0 "/sys/devices/platform/coretemp.0/temp1_input"
// actually temp*_input for *=1,2,3, but I'll just use 1

// Display format strings:
//  Defaults make extensive use of escape characters for colors which require
//  colorstatus patch.  There are also "extended" characters selected to work
//  with terminus2 font for icons.

// Music player daemon
#define MPD_STR      "\x04Î %.50s \x09Ý "   // mpd playing
#define MPD_P_STR    "\x04Î Paused \x09Ý "  //  "  paused
#define MPD_S_STR    "\x04Î Stopped \x09Ý " //  "  stopped
#define MPD_NONE_STR "\x03Î Failed \x09Ý "  //  "  not running

// Volume
#define VOL_STR      "\x07Ô %d%% \x09Ý " // volume in dB
#define VOL_MUTE_STR "\x03Ô %d%% \x09Ý " // muted

// Core load, temperature, and memory usage
#define CPU_STR      "\x01Ï %02.f %02.f %02.f  " // cpu load
#define CPU_HI_STR   "\x03Ï %02.f %02.f %02.f  " // cpu load urgent > CPU_HI
#define TEMP_STR     "\x01Ç %dC  "          // core temp (C)
#define TEMP_HI_STR  "\x03Ç %dC  "          // core temp urgent > CORETEMP_HI
#define MEM_STR      "\x01Þ %dM \x09Ý\x01 " // memory usage

// Wireless usage (wlan0)
#define WIFI_STR     "\x07Ð %-4s \x06Ñ %-3s \x09Ý\x01 " // wlan0 down/up rates

// Battery status, level, time remaining
#define BAT_STR      "\x0AÁ %d%% (%02.2f) " // battery discharging & time left
#define BAT_LOW_STR  "\x03Á %d%% (%02.2f) " //    "    urgent if below BAT_LOW
#define BAT_CHRG_STR "\x0AÅ %d%% "          //    "    charging
#define BAT_FULL_STR "\x0AÀ %d%% "          //    "    full

// Date-time
#define DATE_TIME_STR "\x02 Õ %a %d %b %H:%M "    // date-time

#endif
