#ifndef _DBH_H
#define _DBH_H

struct dbh;

#include "dbt.h"

void dbhload();
struct dbh *dbhnew(unsigned char *sha,size_t si);
struct dbh *dbhget(unsigned char *sha);
void dbhadd(struct dbh *dh,struct dbt *dt,struct dbf *df);
struct dbh *dbhgetnxt(struct dbh *dh);

unsigned char *dbhgetsha(struct dbh *dh);
size_t dbhgetsi(struct dbh *dh);
void dbhsetsi(struct dbh *dh,size_t si);
char *dbhgetmk(struct dbh *dh);

enum dbhex { DE_IN=0x1, DE_EX=0x2, DE_OLD=0x4 };
enum dbhex dbhexdt(struct dbh *dh,struct dbt *dt);
enum dbhex dbhexdf(struct dbh *dh,struct dbf *df);

#endif
