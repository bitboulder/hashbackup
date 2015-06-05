#ifndef _FQ_H
#define _FQ_H

#include "dbt.h"

void fqadd(struct dbf *df);
void fqrun(struct dbt *dt,char (*fana)(struct dbt *dt,struct dbf *df),void (*fwrk)(struct dbf *df));

#endif
