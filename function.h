static Display *dpy;

void setstatus(char *str) {
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}

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
		exit(1);
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
		return(1);
	}
	else
		return(0);
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
		exit(1);
	}

	if (!strftime(buf, sizeof(buf)-1, DATE_TIME_STR, timtm)) {
		fprintf(stderr, "strftime == 0\n");
		exit(1);
	}

	return smprintf("%s", buf);
}

char *loadavg(void) {
	double avgs[3];

	if (getloadavg(avgs, 3) < 0) {
		perror("getloadavg");
		exit(1);
	}

	return smprintf(CPU_STR, 100*avgs[0], 100*avgs[1], 100*avgs[2]);
}

char *memory(void) {
	FILE *f;
	int total, free, buffers, cached, used;
	if(!(f = fopen(MEM_FILE, "r"))) {
		fprintf(stderr, "cannot read %s\n", MEM_FILE);
		exit(1);
	}
  fscanf(f, "MemTotal:%d kB\n", &total);
  fscanf(f, "MemFree:%d kB\n",  &free);
  fscanf(f, "Buffers:%d kB\n",  &buffers);
  fscanf(f, "Cached:%d kB\n",   &cached);
  fclose(f);
  used = (total - free - buffers - cached)/1024;
  return smprintf(MEM_STR, used);
}

char *coretemp(void) {
	FILE *f;
	int temp;

	f = fopen(CPU_TEMP0, "r");
	fscanf(f, "%d", &temp);
	fclose(f);

	temp /= 1000;
  if (temp >= TEMP_HI)
    return smprintf(TEMP_HI_STR, temp);
  else
    return smprintf(TEMP_STR, temp);
}

long rx_old,tx_old,rx_new,tx_new;

char *network(void) {
  FILE *f;
  char rxk[7], txk[7];
  f = fopen(WIFI_DN,"r");
  fscanf(f, "%ld", &rx_new);
  fclose(f);
  f = fopen(WIFI_UP,"r");
  fscanf(f, "%ld", &tx_new);
  fclose(f);
  sprintf(rxk,"%dK",(int)(rx_new-rx_old)/1024/INTERVAL);
  sprintf(txk,"%dK",(int)(tx_new-tx_old)/1024/INTERVAL);
  return smprintf(WIFI_STR, rxk, txk);
}

char *battery(void) {
  FILE *f;
  char state[20];
  int percent;
  float timeleft;
  long now, full, power;

  f = fopen(BATT_NOW,"r");
  fscanf(f,"%ld", &now);
  fclose(f);
  f = fopen(BATT_FULL,"r");
  fscanf(f,"%ld", &full);
  fclose(f);
  f = fopen(BATT_STAT,"r");
  fscanf(f,"%s", state);
  fclose(f);

  percent = now*100/full;
  if (strncmp(state, "Charging", 8) == 0) {
    return smprintf(BAT_CHRG_STR, percent);
  }
  else if (strncmp(state, "Full", 8) == 0) {
    return smprintf(BAT_FULL_STR, percent);
  }
  else {
		f = fopen(BATT_POW,"r");
		fscanf(f,"%ld",&power);
		fclose(f);
		timeleft = (float) now/power;
		if (percent < BATT_LOW)
      return smprintf(BAT_LOW_STR, percent, timeleft);
    else
      return smprintf(BAT_STR, percent, timeleft);
  }
}
