#ifndef _EX_H
#define _EX_H

struct ex;

struct ex *exload(const char *fn);
char exfn(struct ex *ex,const char *fn);

#endif
