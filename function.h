/* xorg */

void setstatus(char *str) {
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}

/* convenience functions */

char *smprintf(char *fmt, ...) {
	va_list fmtargs;
	char *ret;
	int len;

	va_start(fmtargs, fmt);
	len = vsnprintf(NULL, 0, fmt, fmtargs);
	va_end(fmtargs);
	ret = malloc(++len);
	if (ret == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	va_start(fmtargs, fmt);
	vsnprintf(ret, len, fmt, fmtargs);
	va_end(fmtargs);
	return ret;
}

int runevery(time_t *ltime, int sec) {
	time_t now = time(NULL);

	if (difftime(now, *ltime) >= sec) {
		*ltime = now;
		return EXIT_FAILURE;
	}
	else
		return EXIT_SUCCESS;
}

/* status line functions */

char *loadavg(void) {
	double avgs[3];

	if (getloadavg(avgs, 3) < 0) {
		perror("getloadavg");
		exit(EXIT_FAILURE);
	}
	return smprintf(CPU_STR, 100*avgs[0], 100*avgs[1], 100*avgs[2]);
}

char *coretemp(void) {
	FILE *f;
	int temp;

	if(!(f = fopen(CPU_TEMP0, "r")))
		return smprintf("");
	fscanf(f, "%d", &temp);
	fclose(f);

	temp /= 1000;
	if (temp >= TEMP_HI)
		return smprintf(TEMP_HI_STR, temp);
	else
		return smprintf(TEMP_STR, temp);
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
	fscanf(f, "Buffers:%d kB\n",  &buffers);
	fscanf(f, "Cached:%d kB\n",   &cached);
	fclose(f);

	used = (total - free - buffers - cached)/1024;
	return smprintf(MEM_STR, used);
}

void network_init(void) {
	FILE *f;

	if(!(f = fopen(WIFI_DN, "r"))) {
		fprintf(stderr, "cannot read %s\n", WIFI_DN);
		exit(EXIT_FAILURE);
	}
	else {
		fscanf(f,"%ld", &rx_old);
		fclose(f);
	}
	if(!(f = fopen(WIFI_UP, "r"))) {
		fprintf(stderr, "cannot read %s\n", WIFI_UP);
		exit(EXIT_FAILURE);
	}
	else {
		fscanf(f,"%ld", &tx_old);
		fclose(f);
	}
}

char *network(void) {
	FILE *f;
	char rxk[7], txk[7];

	if(!(f = fopen(WIFI_DN, "r")))
		return smprintf("");
	else {
		fscanf(f, "%ld", &rx_new);
		fclose(f);
	}
	if(!(f = fopen(WIFI_UP, "r")))
		return smprintf("");
	else {
		fscanf(f, "%ld", &tx_new);
		fclose(f);
	}

	sprintf(rxk, "%dK", (int)(rx_new-rx_old)/1024/INTERVAL);
	sprintf(txk, "%dK", (int)(tx_new-tx_old)/1024/INTERVAL);
	rx_old = rx_new;
	tx_old = tx_new;
	return smprintf(WIFI_STR, rxk, txk);
}

char *battery(void) {
	FILE *f = NULL;
	long now, full, power;
	char status[11];
	float timeleft;
	int capacity;

	if(!(f = fopen(BATT_NOW, "r")))
		return smprintf("");
	fscanf(f, "%ld", &now);
	fclose(f);
	if(!(f = fopen(BATT_FULL, "r")))
		return smprintf("");
	fscanf(f, "%ld", &full);
	fclose(f);
	if(!(f = fopen(BATT_STAT, "r")))
		return smprintf("");
	fscanf(f, "%s", status);
	fclose(f);
	if(!(f = fopen(BATT_CAP, "r")))
		return smprintf("");
	fscanf(f, "%d", &capacity);
	fclose(f);

	if (strncmp(status, "Charging", 8) == 0)
		return smprintf(BAT_CHRG_STR, capacity);
	else if (strncmp(status, "Full", 8) == 0 || strncmp(status, "Unknown", 8) == 0)
		return smprintf(BAT_FULL_STR, capacity);
	else {
		if (!(f = fopen(BATT_POW,"r")))
			return smprintf("");
		fscanf(f, "%ld", &power);
		fclose(f);
		timeleft = (float) now/power;
		if (capacity < BATT_LOW)
			return smprintf(BAT_LOW_STR, capacity, timeleft);
		else
			return smprintf(BAT_STR, capacity, timeleft);
	}
}

char *mktimes(void) {
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

	tmsleep = 60 - timtm->tm_sec;
	return smprintf("%s", buf);
}

char *music(struct mpd_connection *conn) {
	if(mpd_connection_get_error(conn)) {
		fprintf(stderr, "mpd connection interrupted\n");
		return smprintf(MPD_NONE_STR);
	}

	mpd_command_list_begin(conn, true);
	mpd_send_status(conn);
	mpd_send_current_song(conn);
	mpd_command_list_end(conn);
	struct mpd_status *status = mpd_recv_status(conn);

	if(status == NULL) {
		fprintf(stderr, "null mpd status\n");
		mpd_response_finish(conn);
		return smprintf(MPD_NONE_STR);
	}
	char *mpdstr = NULL;
	switch(mpd_status_get_state(status)) {
		case MPD_STATE_PLAY:
			mpd_response_next(conn);
			struct mpd_song *song = mpd_recv_song(conn);
			const char *title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
			if (!title) title = mpd_song_get_tag(song, MPD_TAG_NAME, 0);
			mpdstr = smprintf(MPD_STR, title);
			mpd_song_free(song);
			break;
		case MPD_STATE_PAUSE:
			mpdstr = smprintf(MPD_P_STR);
			break;
		case MPD_STATE_STOP:
			mpdstr = smprintf(MPD_S_STR);
			break;
		case MPD_STATE_UNKNOWN:
			mpdstr = smprintf(MPD_NONE_STR);
			fprintf(stderr, "mpd state unknown\n");
	}
	mpd_status_free(status);
	mpd_response_finish(conn);
	return mpdstr;
}

char *new_mail(char *dir) {
	int n = 0;
	DIR *d = NULL;
	struct dirent *rf = NULL;

	if(!(d = opendir(dir))) {
		fprintf(stderr, "cannot read directory %s\n", dir);
		exit(EXIT_FAILURE);
	}
	while ((rf = readdir(d)) != NULL) {
		if (strcmp(rf->d_name, ".") != 0 && strcmp(rf->d_name, "..") != 0)
			n++;
	}
	closedir(d);
	if (n == 0)
		return smprintf("");
	else
		return smprintf(MAIL_STR, n);
}
