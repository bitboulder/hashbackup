#ifndef _FNSORT_H
#define _FNSORT_H

struct fns;

struct fns *fnsinit();
void fnsadd(struct fns *fns,const char *fn,char cp,void *arg);
const char *fnsnxt(struct fns *fns,void **arg);

#endif
