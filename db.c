#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <zlib.h>

#include "db.h"
#include "main.h"
#include "help.h"
#include "dat.h"

#define HNCH	8192

#define VERSION	1
#define MARKER	0x4381E32F

struct dbf {
	struct dbf *nxt;
	char fn[FNLEN];
	char lnk[FNLEN];
	struct st st;
	struct dbh *dh;
	char mk;
};

struct dbt {
	struct dbt *nxt;
	time_t t;
	struct dbf *fhsh[HNCH];
};

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
};

struct db {
	char bdir[FNLEN];
	struct dbt *dt;
	struct dbh *dh[HNCH];
} db = {
	.dt=NULL,
	.dh={NULL},
};

int fkey(const char *fn){
	return 0;
}

char *fnrmnewline(char *fn){
	int i;
	for(i=0;i<FNLEN-1 && fn[i] && fn[i]!='\n';) i++;
	fn[i]='\0';
	return fn;
}

void dbload(){
	FILE *fd;
	DIR *dd;
	struct dirent *di;
	gzFile gd;
	if(!(fd=fopen("basedir","r"))) return;
	if(!fgets(db.bdir,FNLEN,fd)) return;
	fclose(fd);
	db.bdir[FNLEN-1]='\0';
	fnrmnewline(db.bdir);
	if(!(dd=opendir(DD))) return;
	while((di=readdir(dd))){
		char fn[FNLEN];
		int i=0;
		unsigned int v;
		time_t t;
		struct dbt *dt;
		char ffn[FNLEN];
		while(i<251 && di->d_name[i]>='0' && di->d_name[i]<='9') i++;
		if(strncmp(di->d_name+i,".dbt",4)) continue;
		di->d_name[i+4]='\0';
		snprintf(fn,FNLEN,DD "/%s",di->d_name);
		if(!(gd=gzopen(fn,"rb"))) error(1,"dbt file not readable: '%s'",fn);
		gzread(gd,&v,sizeof(unsigned int)); if(v!=MARKER)  error(1,"dbt file with wrong marker: '%s'",fn);
		gzread(gd,&v,sizeof(unsigned int)); if(v!=VERSION) error(1,"dbt file with wrong version: '%s'",fn);
		gzread(gd,&t,sizeof(time_t));
		dt=dbtnew(t);
		while(gzgets(gd,ffn,FNLEN) && ffn[0]!='\n' && ffn[0]){
			struct dbf *df=dbfnew(dt,fnrmnewline(ffn));
			gzread(gd,&df->st,sizeof(struct st));
			switch(df->st.mode){
			case MS_FILE: {
				unsigned char sha[SHALEN];
				gzread(gd,sha,sizeof(unsigned char)*SHALEN);
				dbhadd(dbhget(sha),dt,df);
			} break;
			case MS_LNK: gzread(gd,df->lnk,sizeof(char)*FNLEN); break;
			case MS_DIR: break;
			case MS_NONE: break;
			}
		}
		gzclose(gd);
	}
	closedir(dd);
}

void dbtsave(struct dbt *dt){
	char fn[FNLEN];
	gzFile fd;
	unsigned int v;
	int ch;
	struct dbf *df;
	snprintf(fn,FNLEN,DD "/%li.dbt",dt->t);
	if(!(fd=gzopen(fn,"wb"))) error(1,"db open failed for '%s'",fn);
	v=MARKER;  gzwrite(fd,&v,sizeof(unsigned int));
	v=VERSION; gzwrite(fd,&v,sizeof(unsigned int));
	gzwrite(fd,&dt->t,sizeof(time_t));
	for(ch=0;ch<HNCH;ch++) for(df=dt->fhsh[ch];df;df=df->nxt){
		gzprintf(fd,"%s\n",df->fn);
		gzwrite(fd,&df->st,sizeof(struct st));
		switch(df->st.mode){
		case MS_FILE: gzwrite(fd,df->dh->sha,sizeof(unsigned char)*SHALEN); break;
		case MS_LNK: gzwrite(fd,df->lnk,sizeof(char)*FNLEN); break;
		case MS_DIR: break;
		case MS_NONE: break;
		}
	}
	gzprintf(fd,"\n");
	gzclose(fd);
}

const char *dbbdir(){ return db.bdir; }

struct dbt *dbtnew(time_t t){
	struct dbt *dt=calloc(1,sizeof(struct dbt));
	dt->t= t ? t : time(NULL);
	dt->nxt=db.dt;
	db.dt=dt;
	return dt;
}

struct dbt *dbtget(time_t t){
	struct dbt *dt=db.dt;
	while(dt && dt->t!=t) dt=dt->nxt;
	return dt;
}

struct dbt *dbtgetnxt(struct dbt *dt){ return dt ? dt->nxt : db.dt; }

struct dbt *dbtgetnewest(){
	struct dbt *dti,*dt;
	dti=dt=dbtgetnxt(NULL);
	while((dti=dbtgetnxt(dti))) if(dbtgett(dti)>dbtgett(dt)) dt=dti;
	return dt;
}

time_t dbtgett(struct dbt *dt){ return dt->t; }

void dbtdel(struct dbt *dt){
	char fn[FNLEN];
	snprintf(fn,FNLEN,DD "/%li.dbt",dt->t);
	unlink(fn);
}

struct dbf *dbfnew(struct dbt *dt,const char *fn){
	struct dbf *df=calloc(1,sizeof(struct dbf));
	int fk=fkey(fn);
	df->nxt=dt->fhsh[fk];
	dt->fhsh[fk]=df;
	memcpy(df->fn,fn,FNLEN);
	return df;
}

struct dbf *dbfget(struct dbt *dt,const char *fn){
	int fk=fkey(fn);
	struct dbf *df=dt->fhsh[fk];
	while(df && strncmp(fn,df->fn,FNLEN)) df=df->nxt;
	return df;
}

struct dbf *dbfgetnxt(struct dbt *dt,struct dbf *df){
	int ch;
	if(df && df->nxt) return df->nxt;
	ch = df ? fkey(df->fn)+1 : 0;
	for(;ch<HNCH;ch++) if(dt->fhsh[ch]) return dt->fhsh[ch];
	return NULL;
}

const char *dbfgetfn(struct dbf *df){ return df->fn; }
struct st *dbfgetst(struct dbf *df){ return &df->st; }
char *dbfgetmk(struct dbf *df){ return &df->mk; }
struct dbh *dbfgeth(struct dbf *df){ return df->dh; }
char *dbfgetlnk(struct dbf *df){ return df->lnk; }

struct dbh *dbhget(unsigned char *sha){
	unsigned int ch=(*(unsigned int*)sha)%HNCH;
	struct dbh *dh=db.dh[ch];
	while(dh && memcmp(dh->sha,sha,SHALEN)) dh=dh->nxt;
	if(dh) return dh;
	dh=malloc(sizeof(struct dbh));
	dh->nxt=db.dh[ch];
	db.dh[ch]=dh;
	memcpy(dh->sha,sha,SHALEN);
	dh->hf=NULL;
	dh->si=datsi(sha);
	return dh;
}

void dbhadd(struct dbh *dh,struct dbt *dt,struct dbf *df){
	struct dbhf *hf=dh->hf;
	df->dh=dh;
	while(hf && hf->df!=df) hf=hf->nxt;
	if(hf) return;
	hf=malloc(sizeof(struct dbhf));
	hf->nxt=dh->hf;
	dh->hf=hf;
	hf->dt=dt;
	hf->df=df;
}

unsigned char *dbhgetsha(struct dbh *dh){ return dh->sha; }
size_t dbhgetsi(struct dbh *dh){ return dh->si; }

char dbhexdt(struct dbh *dh,struct dbt *dt){
	struct dbhf *hf;
	if(!dh) return 1;
	for(hf=dh->hf;hf;hf=hf->nxt) if(hf->dt!=dt) return 0;
	return 1;
}
