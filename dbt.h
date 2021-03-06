#ifndef _DBT_H
#define _DBT_H

#include <time.h>

#define DD	"dbt"
#define DH	"dbh"

struct dbf;
struct dbt;

#include "dbh.h"
#include "ext2.h"
#include "str.h"

void dbload();
void dbtsave(struct dbt *dt);

const char *dbbdir();
struct ex *dbgetex();

struct dbt *dbtnew(time_t t);
struct dbt *dbtget(time_t t);
struct dbt *dbtgetnxt(struct dbt *dt);
struct dbt *dbtgetnewest();
time_t dbtgett(struct dbt *dt);
struct dbf *dbtgetc(struct dbt *dt);
void dbtsetc(struct dbt *dt);
void dbtdel(struct dbt *dt);

struct dbf *dbfnew(struct dbt *dt,const struct str fn);
struct dbf *dbfget(struct dbt *dt,const char *fn);
struct dbf *dbfgetnxt(struct dbt *dt,struct dbf *df);
void dbfsetext2(struct dbf *df,struct dbe *de);
struct dbe *dbfgetext2(struct dbf *df);
const char *dbfgetfn(struct dbf *df);
struct st *dbfgetst(struct dbf *df);
char *dbfgetmk(struct dbf *df);
struct dbh *dbfgeth(struct dbf *df);
void dbfseth(struct dbf *df,struct dbh *dh);
struct str *dbfgetlnk(struct dbf *df);
struct dbf *dbfgetc(struct dbf *df);
struct dbf *dbfgetcnxt(struct dbf *df);
size_t dbfgetsi(struct dbf *df);

#endif
