#ifndef _HELP_H
#define _HELP_H

#include <stdint.h>
#include <time.h>
#include "ex.h"

struct st {
	enum fmode { MS_NONE, MS_FILE, MS_DIR, MS_LNK } mode;
	uint32_t uid,gid;
	size_t size;
	time_t mtime,ctime;
};

int dirrec(const char *bdir,struct ex *ex,const char *dir,int (*fnc)(const char*,enum fmode,void *),void *arg);
char *fnrmnewline(char *fn);
struct dbt *timeparse(const char *stime);
const char *timefmt(time_t t);
const char *sizefmt(size_t si);

char statget(char bdir,const char *fn,struct st *st);
void statset(struct st *st,const char *fn);
size_t filesize(const char *fn);
void mkd(const char *fn);

void lnkget(const char *fn,char *lnk);
void lnkset(const char *lnk,const char *fn);

void shaget(const char *fn,unsigned char *sha);
void shagetdb(const char *fn,unsigned char *sha);
void shastr(const char *str,unsigned char *sha);
void fn2sha(const char *fn,unsigned char *sha);
void sha2fn(const unsigned char *sha,char *fn);

#endif
