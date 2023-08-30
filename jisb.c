/* Made by Jumps Are Op. (jumpsareop@gmail.com)
 * Licensed under GPLv3, see LICENSE file for more information.
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <wchar.h>
#include <wctype.h>
typedef wchar_t wchar;
typedef wint_t wint;
typedef size_t uint;

#include <X11/Xlib.h>


#define LEN(x) (sizeof(x)/sizeof(*x))
#define every (i = 0;
#define block i < LEN(blocks); i++)
#define BLK(i,sig,t,s,f,pre,su) { i, BLK##t, sig, pre, su, s, f, 0, 0, { 0 } }
#define BLKBUFSIZ 128

enum BlkType{
	BLKFILE, BLKFUNC, BLKSH
};

/* If `f` is NULL, `s` is used on `type`.
 * If `s` is NULL, `f` is called with the buffer.
 * If both `s` and `f` are non-NULL, and `type` is not FUNC,
 * `f` is ablied to the result of `s`.
 */
struct Block{
	unsigned interval, type;
	int sig;
	const wchar *prefix, *suffix;
	char *s;
	void (*f)(wchar[]);
	char iserror, isempty;
	wchar buf[BLKBUFSIZ+1];
};

uint totalsiz;
wchar *totalbuf;
Display *dpy;

#include "config.h"

void handler(int);
void execblk(int);
void readfile(struct Block*, FILE*(*)(const char*,const char*),
                             int(*)(FILE*));

int main(int argc, char *argv[]){
	unsigned i, unslept = 0, totaltime;
	struct sigaction sa;
	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	if(argc != 1 || !*argv){
		puts("jisb  Jisb Is a Suck-less Bar\nUsage: jisb &");
		return 1;
	}

	if(!(dpy = XOpenDisplay(NULL)))
		return 1;

	totalsiz = (BLKBUFSIZ+1)*LEN(blocks);
	for every block{
		if(blocks[i].prefix)
			totalsiz += wcslen(blocks[i].prefix);
		if(blocks[i].suffix)
			totalsiz += wcslen(blocks[i].suffix);
	}
	totalbuf = (wchar*)malloc(totalsiz*sizeof(wchar));

	for every block{
		if(blocks[i].sig &&
			sigaction(SIGRTMIN+blocks[i].sig, &sa, NULL) == -1)
			perror("sigaction");
		execblk(i);
	}

	do{
		while(unslept)
			unslept = sleep(unslept);
		for every block
			if(blocks[i].interval &&
				!(totaltime%blocks[i].interval))
				execblk(i);
		updateoutput();
		totaltime += interval;
		unslept = sleep(unslept ? unslept : interval);
	}while(1);
}

void handler(int x){
	unsigned i;
	x -= SIGRTMIN;
	for every block
		if(blocks[i].sig == x){
			execblk(i);
			updateoutput();
			return;
		}
}

void execblk(int x){
	struct Block *b = &blocks[x];
	wmemset(b->buf, L'\0', BLKBUFSIZ);
	switch(b->type){
		case BLKFILE: readfile(b, fopen, fclose); break;
		case BLKSH: readfile(b, popen, pclose); break;
	}
	if(b->f)
		b->f(b->buf);
}

void readfile(struct Block *b, FILE*(*o)(const char*,const char*),
                               int(*c)(FILE*)){
	FILE *fp = o(b->s, "r");
	char tmp[BLKBUFSIZ] = {0};
	if(!fp)
		return;
	if(fwide(fp, 1) > 0)
		fgetws(b->buf, BLKBUFSIZ, fp);
	else if(read(fileno(fp), &tmp, BLKBUFSIZ))
		mbstowcs(b->buf, tmp, BLKBUFSIZ);
	c(fp);
}
