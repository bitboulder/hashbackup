#ifndef _DBH_H
#define _DBH_H

struct dbh;

#include "db.h"

void dbhload();
struct dbh *dbhnew(unsigned char *sha,size_t si);
struct dbh *dbhget(unsigned char *sha);
void dbhadd(struct dbh *dh,struct dbt *dt,struct dbf *df);
struct dbh *dbhgetnxt(struct dbh *dh);

unsigned char *dbhgetsha(struct dbh *dh);
size_t dbhgetsi(struct dbh *dh);
char *dbhgetmk(struct dbh *dh);
void dbhresetmk();

char dbhexdt(struct dbh *dh,struct dbt *dt);

#endif
