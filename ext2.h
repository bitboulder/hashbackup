#ifndef _EXT2_H
#define _EXT2_H

struct dbe;

#include "dbt.h"

char ext2read(struct dbf *df);
void ext2restore(struct dbf *df,const char *fn);
struct dbe *ext2load(void *fd);
void ext2save(struct dbe *de,void *fd);
size_t ext2getsi(struct dbe *de);
void ext2list(struct dbe *de);

#endif
