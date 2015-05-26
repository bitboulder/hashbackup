#ifndef _DB_H
#define _DB_H

#define FNLEN	1024

void dbload(const char *fn);
void dbsave(const char *fn);

struct dbt *dbtnew(time_t t);
struct dbt *dbtgetnxt(struct dbt *dt);
time_t dbtgett(struct dbt *dt);

struct dbf *dbfnew(struct dbt *dt,const char *fn);
struct dbf *dbfgetnxt(struct dbt *dt,struct dbf *df);

#endif
