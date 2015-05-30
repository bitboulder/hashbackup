#ifndef _DIRREC_H
#define _DIRREC_H

#include "help.h"

int dirrec(const char *bdir,struct ex *ex,const char *dir,int (*fnc)(const char*,enum ftyp,void *),void *arg);

#endif
