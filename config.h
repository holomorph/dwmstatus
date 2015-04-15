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

#define CLR(format, color) color format

#define SEPARATOR    " " CLR("|", "\x09") " "

/* mail */
#define MAIL_FMT     CLR("\uE013 %d", "\x01")

/* volume level */
#define VOL_FMT      CLR("\uE010 %d%s", "\x07")
#define VOL_MUTE_FMT CLR("\uE010 %d%s", "\x03")

/* load avgs, core temperature, & memory usage */
#define CPU_FMT      CLR("\uE006 %.2f %.2f %.2f", "\x01")
#define CPU_HI_FMT   CLR("\uE006 %.2f %.2f %.2f", "\x03")
#define TEMP_FMT     CLR("\uE007 %dC", "\x01")
#define TEMP_HI_FMT  CLR("\uE007 %dC", "\x03")
#define MEM_FMT      CLR("\uE008 %dM", "\x01")

/* network */
#define RX_FMT       CLR("\uE009 %-4s", "\x07")
#define TX_FMT       CLR("\uE00A %-3s", "\x06")
#define NET_FMT      CLR("\uE015", "%s")

/* battery status, level, & time remaining */
#define BAT_FMT      CLR("\uE00D %.1f%% (%02.2f) %2.2fW", "\x0A")
#define BAT_LOW_FMT  CLR("\uE00B %.1f%% (%02.2f) %2.2fW", "\x03")
#define BAT_CHRG_FMT CLR("\uE00E %.1f%%", "\x0A")
#define BAT_FULL_FMT CLR("\uE00F %.1f%%", "\x0A")

/* date */
#define TIMEDATE_FMT CLR("%a %d %b %H:%M", "\x05")
