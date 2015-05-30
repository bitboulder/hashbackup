#ifndef _DB_H
#define _DB_H

#include <time.h>

#define DD	"db"
#define DH	"dat"

struct dbf;
struct dbt;
struct dbh;

void dbload();
void dbtsave(struct dbt *dt);

const char *dbbdir();
struct ex *dbgetex();

struct dbt *dbtnew(time_t t);
struct dbt *dbtget(time_t t);
struct dbt *dbtgetnxt(struct dbt *dt);
struct dbt *dbtgetnewest();
time_t dbtgett(struct dbt *dt);
void dbtdel(struct dbt *dt);

struct dbf *dbfnew(struct dbt *dt,const char *fn);
struct dbf *dbfget(struct dbt *dt,const char *fn);
struct dbf *dbfgetnxt(struct dbt *dt,struct dbf *df);
const char *dbfgetfn(struct dbf *df);
struct st *dbfgetst(struct dbf *df);
char *dbfgetmk(struct dbf *df);
struct dbh *dbfgeth(struct dbf *df);
char *dbfgetlnk(struct dbf *df);

struct dbh *dbhget(unsigned char *sha);
void dbhadd(struct dbh *dh,struct dbt *dt,struct dbf *df);
unsigned char *dbhgetsha(struct dbh *dh);
size_t dbhgetsi(struct dbh *dh);
void dbhsetsi(struct dbh *dh,size_t si);
char dbhexdt(struct dbh *dh,struct dbt *dt);

#endif
