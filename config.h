#define BATT_LOW 11 /* Upper threshold for urgent battery percent */
#define LOAD_HI  1  /* Lower threshold for urgent cpu load */
#define TEMP_HI  75 /* Lower threshold for urgent core temp */
#define INTERVAL 2  /* Sleeps for INTERVAL seconds each iter */

/* files read for system info */
#define MEM_FILE  "/proc/meminfo"
#define BATT_POW  "/sys/class/power_supply/BAT1/power_now"
#define BATT_NOW  "/sys/class/power_supply/BAT1/energy_now"
#define BATT_FULL "/sys/class/power_supply/BAT1/energy_full"
#define BATT_STAT "/sys/class/power_supply/BAT1/status"
#define BATT_CAP  "/sys/class/power_supply/BAT1/capacity"
#define CPU_TEMP0 "/sys/devices/platform/coretemp.0/temp1_input"

/* mail */
#define MAIL_STR     "\x0A\uE013 %d \x09| "

/* mpd */
#define MPD_STR      "\x04\uE012 %.50s \x09| "
#define MPD_P_STR    "\x04\uE012 Paused \x09| "
#define MPD_S_STR    "\x04\uE012 Stopped \x09| "
#define MPD_NONE_STR "\x03\uE012 Failed \x09| "

/* volume via pulseaudio */
#define VOL_STR      "\x07\uE010 %d%% \x09| "
#define VOL_MUTE_STR "\x03\uE010 %d%% \x09| "

/* load avgs, core temperature, & memory usage */
#define CPU_STR      "\x01\uE006 %02.f %02.f %02.f  "
#define CPU_HI_STR   "\x03\uE006 %02.f %02.f %02.f  "
#define TEMP_STR     "\x01\uE007 %dC  "
#define TEMP_HI_STR  "\x03\uE007 %dC  "
#define MEM_STR      "\x01\uE008 %dM \x09|\x01 "

/* network rates */
#define WIFI_STR     "\x07\uE009 %-4s \x06\uE00A %-3s \x09|\x01 "

/* battery status, level, & time remaining */
#define BAT_STR      "\x0A\uE00D %d%% (%02.2f) "
#define BAT_LOW_STR  "\x03\uE00B %d%% (%02.2f) "
#define BAT_CHRG_STR "\x0A\uE00E %d%% "
#define BAT_FULL_STR "\x0A\uE00F %d%% "

/* date */
#define DATE_TIME_STR "\x05 \uE011 %a %d %b %H:%M"
