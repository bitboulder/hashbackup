#ifndef _HELP_H
#define _HELP_H

#include <stdint.h>

struct st {
	enum { MS_NONE, MS_FILE, MS_DIR, MS_LNK } mode;
	uint32_t uid,gid;
	size_t size;
	time_t mtime,ctime;
};

int dirrec(const char *bdir,const char *dir,int (*fnc)(const char*,void *),void *arg);
struct dbt *timeparse(const char *stime);
const char *timefmt(time_t t);
const char *sizefmt(size_t si);
char statget(const char *fn,struct st *st);
void statset(struct st *st,const char *fn);
void lnkget(const char *fn,char *lnk);
void lnkset(const char *lnk,const char *fn);
char shaget(const char *fn,unsigned char *sha);

#endif
