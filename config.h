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

/* mail */
#define MAIL_FMT     "\x01\uE013 %d"

/* volume level */
#define VOL_FMT      "\x07\uE010 %d%s"
#define VOL_MUTE_FMT "\x03\uE010 %d%s"

/* load avgs, core temperature, & memory usage */
#define CPU_FMT      "\x01\uE006 %.2f %.2f %.2f"
#define CPU_HI_FMT   "\x03\uE006 %.2f %.2f %.2f"
#define TEMP_FMT     "\x01\uE007 %dC"
#define TEMP_HI_FMT  "\x03\uE007 %dC"
#define MEM_FMT      "\x01\uE008 %dM"

/* network */
#define RX_FMT       "\x07\uE009 %-4s"
#define TX_FMT       "\x06\uE00A %-3s"
#define NET_FMT      "%s\uE015"

/* battery status, level, & time remaining */
#define BAT_FMT      "\x0A\uE00D %.1f%% (%02.2f) %2.2fW"
#define BAT_LOW_FMT  "\x03\uE00B %.1f%% (%02.2f) %2.2fW"
#define BAT_CHRG_FMT "\x0A\uE00E %.1f%%"
#define BAT_FULL_FMT "\x0A\uE00F %.1f%%"

/* date */
#define TIMEDATE_FMT "\x05%a %d %b %H:%M"
