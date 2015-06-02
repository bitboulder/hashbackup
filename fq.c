#include <stdio.h>
#include <stdlib.h>

#include "fq.h"
#include "main.h"

struct fq {
	struct fq *nxt;
	struct dbf *df;
} *fq=NULL, *fql=NULL;

void fqadd(struct dbf *df){
	struct fq *fi=malloc(sizeof(struct fq));
	fi->df=df;
	fi->nxt=NULL;
	if(fql){ fql->nxt=fi; fql=fi; }else fq=fql=fi;
}

void fqrun(struct dbt *dt,void (*fnc)(struct dbt *dt,struct dbf *df)){
	for(;fq;fq=fq->nxt) fnc(dt,fq->df);
	fql=NULL;
}


