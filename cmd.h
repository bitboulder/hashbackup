#ifndef _CMD_H
#define _CMD_H

void init(const char *basedir,const char *exclude);
void tlist();
void flist(const char *stime,char ext2);
void diff(const char *stime,char noctime,char sha);
void commit(char noctime);
void restore(const char *fn,const char *stime,const char *dstdir);
void del(const char *stime);
void dbcheck();

#endif
