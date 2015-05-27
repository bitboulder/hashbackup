#ifndef _HELP_H
#define _HELP_H

void dirrec(const char *bdir,const char *dir,void (*fnc)(const char*,void *),void *arg);
struct dbt *timeparse(const char *stime);
char mstat(const char *fn,struct stat *st);
char msha(const char *fn,unsigned char *sha);

#endif
