#ifndef _FQ_H
#define _FQ_H

#include "db.h"

void fqadd(struct dbf *df);
void fqrun(struct dbt *dt,void (*fnc)(struct dbt *dt,struct dbf *df));

#endif
