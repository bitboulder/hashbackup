#ifndef _CMD_H
#define _CMD_H

void init(const char *basedir);
void diff(const char *stime);
void commit();
void dbcheck();
void tlist();
void flist(const char *stime);
void del(const char *stime);

#endif
