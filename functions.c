/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <err.h>
#include <errno.h>

#include <ifaddrs.h>
#include <netdb.h>
#include <net/if.h>

#include "config.h"
#include "functions.h"

void loadavg(buffer_t *buf) {
	double avgs[3];
	unsigned int hi;

	if (getloadavg(avgs, 3) < 0) {
		perror("getloadavg");
		exit(EXIT_FAILURE);
	}
	hi = (avgs[0] > LOAD_HI);
	buffer_printf(buf, hi ? CPU_HI_STR : CPU_STR, avgs[0], avgs[1], avgs[2]);
}

void coretemp(buffer_t *buf) {
	FILE *f;
	int temp;
	unsigned int hi;

	if(!(f = fopen(CPU_TEMP0, "r"))) {
		buffer_clear(buf);
		return;
	}
	fscanf(f, "%d", &temp);
	fclose(f);

	temp /= 1000;
	hi = (temp >= TEMP_HI);
	buffer_printf(buf, hi ? TEMP_HI_STR : TEMP_STR, temp);
}

void memory(buffer_t *buf) {
	FILE *f;
	int total, free, buffers, cached, used;

	if(!(f = fopen(MEM_FILE, "r")))
		err(errno, "cannot read %s", MEM_FILE);
	fscanf(f, "MemTotal:%d kB\n", &total);
	fscanf(f, "MemFree:%d kB\n",  &free);
	fscanf(f, "MemAvailable:%*d kB\n");
	fscanf(f, "Buffers:%d kB\n",  &buffers);
	fscanf(f, "Cached:%d kB\n",   &cached);
	fclose(f);

	used = (total - free - buffers - cached)/1024;
	buffer_printf(buf, MEM_STR, used);
}

void *network_init(const char *ifname) {
	Interface *p = malloc(sizeof(Interface));
	if(!p)
		err(errno, "iface malloc");
	p->name = ifname;
	p->rx = smprintf("/sys/class/net/%s/statistics/rx_bytes", ifname);
	p->tx = smprintf("/sys/class/net/%s/statistics/tx_bytes", ifname);
	FILE *f;

	if(!(f = fopen(p->rx, "r"))) {
		network_deinit(p);
		err(errno, "%s", ifname);
	}
	fscanf(f,"%ld", &p->rx_bytes);
	fclose(f);
	if(!(f = fopen(p->tx, "r"))) {
		network_deinit(p);
		err(errno, "%s", ifname);
	}
	fscanf(f,"%ld", &p->tx_bytes);
	fclose(f);
	return p;
}

void network_deinit(Interface *iface) {
	free(iface->rx);
	free(iface->tx);
	free(iface);
}

void network(buffer_t *buf, Interface *iface, long rx_old, long tx_old) {
	FILE *f;
	char rxk[7], txk[7];

	if(!(f = fopen(iface->rx, "r"))) {
        buffer_clear(buf);
		return;
    }
	fscanf(f, "%ld", &iface->rx_bytes);
	fclose(f);
	if(!(f = fopen(iface->tx, "r"))) {
        buffer_clear(buf);
		return;
    }
	fscanf(f, "%ld", &iface->tx_bytes);
	fclose(f);

	sprintf(rxk, "%.1f", (float)(iface->rx_bytes-rx_old)/1024/INTERVAL);
	sprintf(txk, "%.1f", (float)(iface->tx_bytes-tx_old)/1024/INTERVAL);
	buffer_printf(buf, NET_STR, rxk, txk);
}

void ipaddr(buffer_t *buf, const char *ifname) {
	struct ifaddrs *ifaddr, *ifa;
	socklen_t len = sizeof(struct sockaddr_in);
	char host[NI_MAXHOST];
	int ret;

	if(getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}

	for(ifa = ifaddr;
		(ifa != NULL && (strcmp(ifa->ifa_name, ifname) != 0 || ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET));
		ifa = ifa->ifa_next) {
		if (strcmp(ifa->ifa_name, ifname) != 0)
			continue;
		if ((ifa->ifa_flags & IFF_RUNNING) == 0) {
			freeifaddrs(ifaddr);
			/* iface is down */
			buffer_printf(buf, NET_ICON, "\x09");
			return;
		}
	}
	if (ifa == NULL) {
		freeifaddrs(ifaddr);
		buffer_clear(buf);
		return;
	}

	memset(host, 0, sizeof(host));
	if ((ret = getnameinfo(ifa->ifa_addr, len, host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST)) != 0) {
		fprintf(stderr, "%s\n", gai_strerror(ret));
		freeifaddrs(ifaddr);
		/* no address */
		buffer_printf(buf, NET_ICON, "\x03");
		return;
	}
	freeifaddrs(ifaddr);
	buffer_printf(buf, NET_ICON, "\x04");
}

void battery(buffer_t *buf) {
	FILE *f = NULL;
	long now, full, power;
	char status[11];
	float capacity, timeleft;
	unsigned int low;

	if(!(f = fopen(BATT_NOW, "r"))) {
		buffer_clear(buf);
		return;
	}
	fscanf(f, "%ld", &now);
	fclose(f);
	if(!(f = fopen(BATT_FULL, "r"))) {
		buffer_clear(buf);
		return;
	}
	fscanf(f, "%ld", &full);
	fclose(f);
	if(!(f = fopen(BATT_STAT, "r"))) {
		buffer_clear(buf);
		return;
	}
	fscanf(f, "%s", status);
	fclose(f);

	capacity = (float) 100*now/full;
	if (strcmp(status, "Charging") == 0)
		buffer_printf(buf, BAT_CHRG_STR, capacity);
	else if (strcmp(status, "Full") == 0 || strcmp(status, "Unknown") == 0)
		buffer_printf(buf, BAT_FULL_STR, capacity);
	else {
		if (!(f = fopen(BATT_POW,"r"))) {
			buffer_clear(buf);
			return;
		}
		fscanf(f, "%ld", &power);
		fclose(f);
		timeleft = (float) now/power;
		low = (capacity < BATT_LOW);
		buffer_printf(buf, low ? BAT_LOW_STR : BAT_STR, capacity, timeleft, (float)1.0e-6*power);
	}
}

void mktimes(buffer_t *buf, int *tmsleep) {
	char tmp[129];
	time_t tim;
	struct tm *timtm;

	memset(tmp, 0, sizeof(tmp));
	tim = time(NULL);
	if (!(timtm = localtime(&tim)))
		err(errno, "localtime");
	if (!strftime(tmp, sizeof(tmp)-1, DATE_TIME_STR, timtm))
		err(errno, "strftime");
	*tmsleep = 60 - timtm->tm_sec;
	buffer_printf(buf, "%s", tmp);
}

char *get_maildir(const char *maildir) {
	if(!maildir || maildir[0] == '\0')
		return NULL;
	return smprintf("%s/%s/new", maildir, MAIL_BOX);
}

void new_mail(buffer_t *buf, const char *maildir) {
	if(maildir == NULL) {
		buffer_clear(buf);
		return;
	}
	int n = 0;
	DIR *d = NULL;
	struct dirent *rf = NULL;

	if(!(d = opendir(maildir)))
		err(errno, "cannot read directory %s", maildir);
	while ((rf = readdir(d)) != NULL) {
		if (strcmp(rf->d_name, ".") != 0 && strcmp(rf->d_name, "..") != 0)
			n++;
	}
	closedir(d);
	if (n == 0) {
		buffer_clear(buf);
		return;
	}
	else
		buffer_printf(buf, MAIL_STR, n);
}
