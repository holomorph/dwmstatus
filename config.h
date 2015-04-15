/* See LICENSE file for copyright and license details. */

#define MAIL_BOX "inbox"

#define BATT_LOW 10 /* Upper threshold for urgent battery percent */
#define LOAD_HI  1  /* Lower threshold for urgent cpu load */
#define TEMP_HI  75 /* Lower threshold for urgent core temp */
#define INTERVAL 10 /* Sleeps for INTERVAL seconds each iteration */

/* files read for system info */
#define MEM_FILE  "/proc/meminfo"
#define BATT_POW  "/sys/class/power_supply/BAT1/power_now"
#define BATT_NOW  "/sys/class/power_supply/BAT1/energy_now"
#define BATT_FULL "/sys/class/power_supply/BAT1/energy_full_design"
#define BATT_STAT "/sys/class/power_supply/BAT1/status"
#define CPU_TEMP0 "/sys/class/hwmon/hwmon0/temp2_input"

/* https://developer.gnome.org/pango/stable/PangoMarkupFormat.html */
#define CLR(format, color) "<span color=\"" color "\">" format "</span>"
/* #define CLR(format, color) color format */
/* #define CLR(format, color) format */

#define SEPARATOR    " " CLR("\u00A7", "#373b41") " "

/* mail */
#define MAIL_FMT     CLR("\u2605 %d", "#facb70")

/* volume level */
#define VOL_FMT      CLR("\u266B %d%s", "#c2cc66")
#define VOL_MUTE_FMT CLR("\u266B %d%s", "#af3a3a")

/* load avgs, core temperature, & memory usage */
#define CPU_FMT      CLR("\u240A %.2f %.2f %.2f", "#facb70")
#define CPU_HI_FMT   CLR("\u240A %.2f %.2f %.2f", "#af3a3a")
#define TEMP_FMT     CLR("\u2646 %dC", "#facb70")
#define TEMP_HI_FMT  CLR("\u2646 %dC", "#af3a3a")
#define MEM_FMT      CLR("\u265E %dM", "#facb70")

/* network */
#define RX_FMT       CLR("\u2193 %-4s", "#c2cc66")
#define TX_FMT       CLR("\u2191 %-3s", "#e06666")
#define NET_FMT      "\u265C"

/* battery status, level, & time remaining */
#define BAT_FMT      CLR("BAT %.1f%% (%02.2f) %2.2fW", "#c69bdd")
#define BAT_LOW_FMT  CLR("LOW %.1f%% (%02.2f) %2.2fW", "#af3a3a")
#define BAT_CHRG_FMT CLR("\u26A1 %.1f%%", "#c69bdd")
#define BAT_FULL_FMT CLR("FULL %.1f%%", "#c69bdd")

/* date */
#define TIMEDATE_FMT CLR("%a %d %b %H:%M", "#efefef")
