/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <dirent.h>

#include <ifaddrs.h>
#include <netdb.h>
#include <net/if.h>

#include "config.h"
#include "functions.h"

char *loadavg(void) {
	double avgs[3];
	unsigned int hi;

	if (getloadavg(avgs, 3) < 0) {
		perror("getloadavg");
		exit(EXIT_FAILURE);
	}
	hi = (avgs[0] > LOAD_HI);
	return smprintf((hi ? CPU_HI_STR : CPU_STR), avgs[0], avgs[1], avgs[2]);
}

char *coretemp(void) {
	FILE *f;
	int temp;
	unsigned int hi;

	if(!(f = fopen(CPU_TEMP0, "r")))
		return NULL;
	fscanf(f, "%d", &temp);
	fclose(f);

	temp /= 1000;
	hi = (temp >= TEMP_HI);
	return smprintf((hi ? TEMP_HI_STR : TEMP_STR), temp);
}

char *memory(void) {
	FILE *f;
	int total, free, buffers, cached, used;

	if(!(f = fopen(MEM_FILE, "r"))) {
		fprintf(stderr, "cannot read %s\n", MEM_FILE);
		exit(EXIT_FAILURE);
	}
	fscanf(f, "MemTotal:%d kB\n", &total);
	fscanf(f, "MemFree:%d kB\n",  &free);
	fscanf(f, "MemAvailable:%*d kB\n");
	fscanf(f, "Buffers:%d kB\n",  &buffers);
	fscanf(f, "Cached:%d kB\n",   &cached);
	fclose(f);

	used = (total - free - buffers - cached)/1024;
	return smprintf(MEM_STR, used);
}

int network_init(Interface *iface, const char *ifname) {
	iface->name = ifname;
	iface->rx = smprintf("/sys/class/net/%s/statistics/rx_bytes", ifname);
	iface->tx = smprintf("/sys/class/net/%s/statistics/tx_bytes", ifname);
	FILE *f;

	if(!(f = fopen(iface->rx, "r"))) {
		fprintf(stderr, "cannot read %s\n", iface->rx);
		return EXIT_FAILURE;
	}
	fscanf(f,"%ld", &iface->rx_bytes);
	fclose(f);
	if(!(f = fopen(iface->tx, "r"))) {
		fprintf(stderr, "cannot read %s\n", iface->tx);
		return EXIT_FAILURE;
	}
	fscanf(f,"%ld", &iface->tx_bytes);
	fclose(f);
	return EXIT_SUCCESS;
}

void network_deinit(Interface *iface) {
	free(iface->rx);
	free(iface->tx);
	free(iface);
}

char *network(Interface *iface, long rx_old, long tx_old) {
	FILE *f;
	char rxk[7], txk[7];

	if(!(f = fopen(iface->rx, "r")))
		return NULL;
	fscanf(f, "%ld", &iface->rx_bytes);
	fclose(f);
	if(!(f = fopen(iface->tx, "r")))
		return NULL;
	fscanf(f, "%ld", &iface->tx_bytes);
	fclose(f);

	sprintf(rxk, "%.1f", (float)(iface->rx_bytes-rx_old)/1024/INTERVAL);
	sprintf(txk, "%.1f", (float)(iface->tx_bytes-tx_old)/1024/INTERVAL);
	return smprintf(NET_STR, rxk, txk);
}

char *ipaddr(const char *ifname) {
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
			return smprintf(NET_ICON, "\x09");
		}
	}
	if (ifa == NULL) {
		freeifaddrs(ifaddr);
		return NULL;
	}

	memset(host, 0, sizeof(host));
	if ((ret = getnameinfo(ifa->ifa_addr, len, host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST)) != 0) {
		fprintf(stderr, "%s\n", gai_strerror(ret));
		freeifaddrs(ifaddr);
		/* no address */
		return smprintf(NET_ICON, "\x03");
	}
	freeifaddrs(ifaddr);
	return smprintf(NET_ICON, "\x04");
}

char *battery(void) {
	FILE *f = NULL;
	long now, full, power;
	char status[11];
	float capacity, timeleft;
	unsigned int low;

	if(!(f = fopen(BATT_NOW, "r")))
		return NULL;
	fscanf(f, "%ld", &now);
	fclose(f);
	if(!(f = fopen(BATT_FULL, "r")))
		return NULL;
	fscanf(f, "%ld", &full);
	fclose(f);
	if(!(f = fopen(BATT_STAT, "r")))
		return NULL;
	fscanf(f, "%s", status);
	fclose(f);

	capacity = (float) 100*now/full;
	if (strcmp(status, "Charging") == 0)
		return smprintf(BAT_CHRG_STR, capacity);
	else if (strcmp(status, "Full") == 0 || strcmp(status, "Unknown") == 0)
		return smprintf(BAT_FULL_STR, capacity);
	else {
		if (!(f = fopen(BATT_POW,"r")))
			return NULL;
		fscanf(f, "%ld", &power);
		fclose(f);
		timeleft = (float) now/power;
		low = (capacity < BATT_LOW);
		return smprintf((low ? BAT_LOW_STR : BAT_STR), capacity, timeleft, (float)1.0e-6*power);
	}
}

char *mktimes(int *tmsleep) {
	char buf[129];
	time_t tim;
	struct tm *timtm;

	memset(buf, 0, sizeof(buf));
	tim = time(NULL);
	timtm = localtime(&tim);
	if (timtm == NULL) {
		perror("localtime");
		exit(EXIT_FAILURE);
	}

	if (!strftime(buf, sizeof(buf)-1, DATE_TIME_STR, timtm)) {
		fprintf(stderr, "strftime == 0\n");
		exit(EXIT_FAILURE);
	}

	*tmsleep = 60 - timtm->tm_sec;
	return smprintf("%s", buf);
}

char *get_maildir(const char *maildir) {
	if(!maildir || maildir[0] == '\0')
		return NULL;
	return smprintf("%s/%s/new", maildir, MAIL_BOX);
}

char *new_mail(const char *maildir) {
	if(maildir == NULL)
		return NULL;
	int n = 0;
	DIR *d = NULL;
	struct dirent *rf = NULL;

	if(!(d = opendir(maildir))) {
		fprintf(stderr, "cannot read directory %s\n", maildir);
		exit(EXIT_FAILURE);
	}
	while ((rf = readdir(d)) != NULL) {
		if (strcmp(rf->d_name, ".") != 0 && strcmp(rf->d_name, "..") != 0)
			n++;
	}
	closedir(d);
	if (n == 0)
		return NULL;
	else
		return smprintf(MAIL_STR, n);
}
