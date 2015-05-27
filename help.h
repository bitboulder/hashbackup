#ifndef _HELP_H
#define _HELP_H

#include <stdint.h>

#define MS_DIR	0x01
#define MS_FILE	0x02
#define MS_LNK 	0x04
struct mstat {
	uint8_t mode;
	uint32_t uid,gid;
	size_t size;
	time_t mtime,ctime;
};

void dirrec(const char *bdir,const char *dir,void (*fnc)(const char*,void *),void *arg);
struct dbt *timeparse(const char *stime);
const char *timefmt(time_t t);
const char *sizefmt(size_t si);
char mstat(const char *fn,struct mstat *st);
void mstatset(struct mstat *st,const char *fn);
char msha(const char *fn,unsigned char *sha);

#endif
