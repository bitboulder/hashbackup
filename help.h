#ifndef _HELP_H
#define _HELP_H

#include <stdint.h>
#include <time.h>
#include "ex.h"
#include "str.h"

struct st {
	enum ftyp { FT_NONE, FT_FILE, FT_DIR, FT_LNK, FT_EXT2 } typ;
	uint32_t uid,gid,mode;
	size_t size;
	time_t atime,mtime,ctime;
};
enum statcmp {
	SD_EQL  =0x000,
	SD_TYP  =0x001,
	SD_UID  =0x002,
	SD_GID  =0x004,
	SD_MODE =0x008,
	SD_SIZE =0x010,
	SD_MTIME=0x020,
	SD_CTIME=0x040,
	SD_SHA  =0x080,
	SD_NEW  =0x100,
	SD_DEL  =0x200,
};

#define MIN(a,b)	((a)<(b)?(a):(b))

struct str *fnrmnewline(struct str *fn);
struct dbt *timeparse(const char *stime);
const char *timefmt(time_t t);
const char *sizefmt(size_t si);
const char *statcmpfmt(enum statcmp sd);

char statget(char bdir,const char *fn,struct st *st);
void statset(struct st *st,const char *fn);
enum statcmp statcmp(struct st *a,struct st *b,char noctime);
size_t filesize(const char *fn);
void mkd(const char *fn);

void lnkget(const char *fn,struct str *lnk);
void lnkset(const char *lnk,const char *fn);

#endif
