#ifndef _CMD_H
#define _CMD_H

void init(const char *basedir,const char *exclude);
void tlist();
void flist(const char *stime);
void diff(const char *stime,char sha);
void commit();
void restore(const char *fn,const char *stime,const char *dstdir);
void del(const char *stime);
void dbcheck();

#endif
