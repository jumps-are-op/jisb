/* Check for intervals every n seconds. */
unsigned interval = 1;

#define DEFFUN(x) void x(wchar b[])
DEFFUN(nmail);
DEFFUN(getcpu);
DEFFUN(getmem);
DEFFUN(gettemp);
DEFFUN(getvol);
DEFFUN(gettime);

#define MAILDIR "/home/jumps/.local/share/mail/INBOX/new/"
#define TEMP "/sys/class/hwmon/hwmon1/device/temp"
#define BAT  "/sys/class/power_supply/BAT0/capacity"
#define AUD  "pamixer --get-volume-human"
#define CPU  "/proc/stat"
struct Block blocks[] = {
	/* interval, RTMIN+signal, type, string, func, prefix, suffix */
	BLK(3, 2, FUNC, NULL, nmail,   NULL, L"m"),
	BLK(1, 0, FUNC, NULL, getcpu,  NULL, L"C"),
	BLK(3, 0, FUNC, NULL, getmem,  NULL, L"M"),
	BLK(1, 0, FILE, TEMP, gettemp, NULL, L"T"),
	BLK(5, 0, FILE, BAT,  NULL,    NULL, L"B"),
	BLK(0, 1, SH,   AUD,  getvol,  NULL, L"V"),
	BLK(5, 0, FUNC, NULL, gettime, NULL, NULL),
};

/* Block functions */
DEFFUN(nmail){
	int n = 0;
	DIR *dir = opendir(MAILDIR);
	struct dirent *dp;
	if(!dir)
		return;
	while((dp = readdir(dir)) != NULL)
		if(dp->d_name[0] != '.')
			n++;
	closedir(dir);
	if(n)
		swprintf(b, BLKBUFSIZ, L"%'d", n);
}

DEFFUN(getcpu){
	static unsigned long long x[7];
	unsigned long long y[7], sum;
	FILE *fp = fopen(CPU, "r");
	if(!fp)
		return;
	memcpy(y, x, sizeof(x));

	if(fscanf(fp, "%*s %llu %llu %llu %llu %llu %llu %llu ",
	          &x[0], &x[1], &x[2], &x[3], &x[4], &x[5], &x[6]) != 7)
		return;

	if(!y[0])
		return;

	sum = (x[0]+x[1]+x[2]+x[3]+x[4]+x[5]+x[6]) -
	      (y[0]+y[1]+y[2]+y[3]+y[4]+y[5]+y[6]);

	if(!sum)
		return;

	swprintf(b, BLKBUFSIZ, L"%llu", (100*((x[0]+x[1]+x[2]+x[5]+x[6])-
	                                (y[0]+y[1]+y[2]+y[5]+y[6])))/sum);
}

DEFFUN(getmem){
	FILE *fp = fopen("/proc/meminfo", "r");
	char *line = NULL, *xline;
	size_t n = 0;
	unsigned total = 0, aval = 0;
	if(!fp)
		return;
	while(getline(&line, &n, fp)){
		xline = strchr(line, ':');
		if(!xline)
			continue;
		*xline = '\0';
		if(!strcasecmp(line, "MemTotal")){
			xline = line;
			while(*line++ != '\0');
			total = strtoul(line, (char**)NULL, 0);
			free(xline);
			break;
		}
	}
	line = NULL;

	/* It's rewind time! (dead joke, I know) */
	rewind(fp);
	while(getline(&line, &n, fp)){
		xline = strchr(line, ':');
		if(!xline)
			continue;
		*xline = '\0';
		if(!strcasecmp(line, "MemAvailable")){
			xline = line;
			while(*line++ != '\0');
			aval = strtoul(line, (char**)NULL, 0);
			free(xline);
			break;
		}
	}
	swprintf(b, BLKBUFSIZ, L"%d", 100-(aval*100/total));
}

DEFFUN(gettemp){
	b[wcslen(b)-4] = L'\0';
}

DEFFUN(getvol){
	if(b[2] == L'%')
		b[2] = L'\0';
	if(b[3] == L'%')
		b[3] = L'\0';
	if(*b == L'm'){
		wcscpy(b, L"**");
		return;
	}
	if(!iswdigit(*b))
		wcscpy(b, L"??");
}

DEFFUN(gettime){
	time_t tim = time(NULL);
	char tmp[BLKBUFSIZ] = {0};
	strftime(tmp, BLKBUFSIZ, "%R %d/%m", localtime(&tim));
	mbstowcs(b, tmp, BLKBUFSIZ);
}

/* output */
void skipcntrl(wchar *buf, unsigned size){
	wchar *read = buf, *write = buf;
	for(; read < buf+size && *read; read++)
		if(iswprint(*read))
			*write++ = *read;
	*write = L'\0';
}

void updateoutput(void){
	wchar *p = totalbuf;
	char *buf;
	uint i, len;
	for every block{
		if(blocks[i].prefix)
			totalbuf = wcpcpy(totalbuf, blocks[i].prefix);
		skipcntrl(blocks[i].buf, BLKBUFSIZ);
		totalbuf = wcpcpy(totalbuf, blocks[i].buf);
		if(blocks[i].suffix)
			totalbuf = wcpcpy(totalbuf, blocks[i].suffix);
		if(i < LEN(blocks)-1)
			*totalbuf++ = L' ';
	}
	*totalbuf = '\0';
	
	len = wcstombs(NULL, p, 0);
	buf = (char*)malloc(len+1);
	wcstombs(buf, p, len);
	XStoreName(dpy, DefaultRootWindow(dpy), buf);
	XFlush(dpy);
	free(buf);
}
