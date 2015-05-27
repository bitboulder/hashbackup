#ifndef _DB_H
#define _DB_H

#include <time.h>

#define DD	"db"
#define DH	"dat"

struct dbf;
struct dbt;

void dbload();
void dbtsave(struct dbt *dt);

const char *dbbdir();

struct dbt *dbtnew(time_t t);
struct dbt *dbtget(time_t t);
struct dbt *dbtgetnxt(struct dbt *dt);
time_t dbtgett(struct dbt *dt);

struct dbf *dbfnew(struct dbt *dt,const char *fn);
struct dbf *dbfgetnxt(struct dbt *dt,struct dbf *df);
const char *dbfgetfn(struct dbf *df);

#endif
