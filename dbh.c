#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dbh.h"
#include "main.h"
#include "help.h"

#define HNCH	8192

struct dbhf {
	struct dbhf *nxt;
	struct dbt *dt;
	struct dbf *df;
};

struct dbh {
	struct dbh *nxt;
	unsigned char sha[SHALEN];
	struct dbhf *hf;
	size_t si;
	char mk;
} *dbh[HNCH]={NULL};

unsigned int hkey(unsigned char *sha){ return (*(unsigned int*)sha)%HNCH; }

int dbhloadh(const char *fn,enum fmode mode,void *arg){
	unsigned char sha[SHALEN];
	if(mode!=MS_FILE) return 0;
	fn2sha(fn,sha);
	dbhnew(sha,filesize(fn));
	return 1;
}

void dbhload(){
	dirrec(".",NULL,DH,dbhloadh,NULL);
}

struct dbh *dbhnew(unsigned char *sha,size_t si){
	unsigned int ch=hkey(sha);
	struct dbh *dh=malloc(sizeof(struct dbh));
	dh->nxt=dbh[ch];
	dbh[ch]=dh;
	memcpy(dh->sha,sha,SHALEN);
	dh->hf=NULL;
	dh->si=si;
	return dh;
}

struct dbh *dbhget(unsigned char *sha){
	unsigned int ch=hkey(sha);
	struct dbh *dh=dbh[ch];
	while(dh && memcmp(dh->sha,sha,SHALEN)) dh=dh->nxt;
	return dh;
}

void dbhadd(struct dbh *dh,struct dbt *dt,struct dbf *df){
	struct dbhf *hf=dh->hf;
	dbfseth(df,dh);
	while(hf && hf->df!=df) hf=hf->nxt;
	if(hf) return;
	hf=malloc(sizeof(struct dbhf));
	hf->nxt=dh->hf;
	dh->hf=hf;
	hf->dt=dt;
	hf->df=df;
}

struct dbh *dbhgetnxt(struct dbh *dh){
	int ch;
	if(dh && dh->nxt) return dh->nxt;
	ch = dh ? hkey(dh->sha)+1 : 0;
	for(;ch<HNCH;ch++) if(dbh[ch]) return dbh[ch];
	return NULL;
}

unsigned char *dbhgetsha(struct dbh *dh){ return dh?dh->sha:NULL; }
size_t dbhgetsi(struct dbh *dh){ return dh->si; }
char dbhgetmk(struct dbh *dh){ return dh->mk; }
void dbhsetmk(struct dbh *dh,char mk){ dh->mk=mk; }

char dbhexdt(struct dbh *dh,struct dbt *dt){
	struct dbhf *hf;
	if(!dh) return 1;
	for(hf=dh->hf;hf;hf=hf->nxt) if(hf->dt!=dt) return 0;
	return 1;
}
