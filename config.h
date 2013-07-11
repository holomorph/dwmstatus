#define BATT_LOW 11 // Upper threshold for urgent battery %
#define LOAD_HI  1  // Lower threshold for urgent cpu load
#define TEMP_HI  75 // Lower threshold for urgent core temp
#define INTERVAL 2  // Sleeps for INTERVAL seconds each iter

// Files read for system info:
#define MEM_FILE  "/proc/meminfo"
#define MAIL_DIR  "/.mutt/mail/inbox/new"
#define BATT_POW  "/sys/class/power_supply/BAT1/power_now"
#define BATT_NOW  "/sys/class/power_supply/BAT1/energy_now"
#define BATT_FULL "/sys/class/power_supply/BAT1/energy_full"
#define BATT_STAT "/sys/class/power_supply/BAT1/status"
#define BATT_CAP  "/sys/class/power_supply/BAT1/capacity"
#define WIFI_DN   "/sys/class/net/enp3s0/statistics/rx_bytes"
#define WIFI_UP   "/sys/class/net/enp3s0/statistics/tx_bytes"
#define CPU_TEMP0 "/sys/devices/platform/coretemp.0/temp1_input"

// Mail
#define MAIL_STR     "\x0A\uE013 %d \x09| "

// Music player daemon
#define MPD_STR      "\x04\uE012 %.50s \x09|"   // mpd playing
#define MPD_P_STR    "\x04\uE012 Paused \x09|"  //  "  paused
#define MPD_S_STR    "\x04\uE012 Stopped \x09|" //  "  stopped
#define MPD_NONE_STR "\x03\uE012 Failed \x09|"  //  "  not running

// Volume
#define VOL_STR      " \x07\uE010 %d%% \x09| " // volume in dB
#define VOL_MUTE_STR " \x03\uE010 %d%% \x09| " // muted

// Core load, temperature, and memory usage
#define CPU_STR      "\x01\uE006 %02.f %02.f %02.f  " // cpu load
#define CPU_HI_STR   "\x03\uE006 %02.f %02.f %02.f  " // cpu load urgent > CPU_HI
#define TEMP_STR     "\x01\uE007 %dC  "          // core temp (C)
#define TEMP_HI_STR  "\x03\uE007 %dC  "          // core temp urgent > CORETEMP_HI
#define MEM_STR      "\x01\uE008 %dM \x09|\x01 " // memory usage

// Wireless usage (wlan0)
#define WIFI_STR     "\x07\uE009 %-4s \x06\uE00A %-3s \x09|\x01 " // wlan0 down/up rates

// Battery status, level, time remaining
#define BAT_STR      "\x0A\uE00D %d%% (%02.2f) " // battery discharging & time left
#define BAT_LOW_STR  "\x03\uE00B %d%% (%02.2f) " //    "    urgent if below BAT_LOW
#define BAT_CHRG_STR "\x0A\uE00E %d%% "          //    "    charging
#define BAT_FULL_STR "\x0A\uE00F %d%% "          //    "    full

// Date-time
#define DATE_TIME_STR "\x05 \uE011 %a %d %b %H:%M"
