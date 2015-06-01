#ifndef _HELP_H
#define _HELP_H

#include <stdint.h>
#include <time.h>
#include "ex.h"

struct st {
	enum ftyp { FT_NONE, FT_FILE, FT_DIR, FT_LNK } typ;
	uint32_t uid,gid,mode;
	size_t size;
	time_t atime,mtime,ctime;
};

char *fnrmnewline(char *fn);
struct dbt *timeparse(const char *stime);
const char *timefmt(time_t t);
const char *sizefmt(size_t si);

char statget(char bdir,const char *fn,struct st *st);
void statset(struct st *st,const char *fn);
char statcmp(struct st *a,struct st *b);
size_t filesize(const char *fn);
void mkd(const char *fn);

void lnkget(const char *fn,char *lnk);
void lnkset(const char *lnk,const char *fn);

#endif
