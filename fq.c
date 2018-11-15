#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "fq.h"
#include "main.h"

struct fq {
	struct fq *nxt;
	struct dbf *df;
} *fq=NULL, *fql=NULL;

typedef void (*fncwrk)(struct dbf *df);

struct fqwrk {
	struct dbf *df;
	char end;
	fncwrk fwrk;
	pthread_t pt;
	pthread_mutex_t mt;
};

void fqadd(struct dbf *df){
	struct fq *fi=malloc(sizeof(struct fq));
	fi->df=df;
	fi->nxt=NULL;
	if(fql){ fql->nxt=fi; fql=fi; }else fq=fql=fi;
}

void *fqwrk(void *arg){
	struct fqwrk *fqw=(struct fqwrk *)arg;
	while(1){
		struct dbf *df;
		while(!fqw->df){
			if(fqw->end) return NULL;
			usleep(100);
		}
		pthread_mutex_lock(&fqw->mt);
		df=fqw->df;
		fqw->df=NULL;
		pthread_mutex_unlock(&fqw->mt);
		fqw->fwrk(df);
	}
}

void fqrun(struct dbt *dt,char (*fana)(struct dbt *dt,struct dbf *df),void (*fwrk)(struct dbf *df)){
	struct fqwrk fqw={.df=NULL,.end=0,.fwrk=fwrk};
	pthread_mutex_init(&fqw.mt,NULL);
	pthread_create(&fqw.pt,NULL,fqwrk,&fqw);
	/* TODO: sort by file pos */
	for(;fq;fq=fq->nxt) if(fana(dt,fq->df)){
		while(fqw.df) usleep(100);
		pthread_mutex_lock(&fqw.mt);
		fqw.df=fq->df;
		pthread_mutex_unlock(&fqw.mt);
	}
	fql=NULL;
	fqw.end=1;
	pthread_join(fqw.pt,NULL);
	pthread_mutex_destroy(&fqw.mt);
}


