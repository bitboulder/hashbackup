#define _BSD_SOURCE
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
#include "sha.h"
#include "dat.h"
#include "ex.h"

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
	struct dbf *c;
	struct dbf *cnxt;
};

struct dbt {
	struct dbt *nxt;
	time_t t;
	struct dbf *fhsh[HNCH];
};

struct db {
	char bdir[FNLEN];
	struct ex *ex;
	struct dbt *dt;
} db = {
	.dt=NULL,
	.ex=NULL,
};

int fkey(const char *fn){
	unsigned char sha[SHALEN];
	shastr(fn,sha);
	return (*(unsigned int*)sha)%HNCH;
}

char dbloadbdir(){
	FILE *fd;
	if(!(fd=fopen("basedir","r"))) return 0;
	if(!fgets(db.bdir,FNLEN,fd)) return 0;
	fclose(fd);
	db.bdir[FNLEN-1]='\0';
	fnrmnewline(db.bdir);
	return 1;
}

void dbload(){
	DIR *dd;
	struct dirent *di;
	gzFile gd;
	printf("[dbload]\n");
	if(!dbloadbdir()) return;
	db.ex=exload("exclude");
	dbhload();
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
			switch(df->st.typ){
			case FT_FILE: {
				unsigned char sha[SHALEN];
				struct dbh *dh;
				gzread(gd,sha,sizeof(unsigned char)*SHALEN);
				dh=dbhget(sha);
				if(!dh){
					char fn[FNLEN];
					sha2fn(sha,fn);
					error(0,"dat file missing: '%s'",fn);
				}else dbhadd(dh,dt,df);
			} break;
			case FT_LNK: gzread(gd,df->lnk,sizeof(char)*FNLEN); break;
			case FT_DIR: break;
			case FT_NONE: break;
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
	printf("[dbtsave]\n");
	snprintf(fn,FNLEN,DD "/%li.dbt",dt->t);
	if(!(fd=gzopen(fn,"wb"))) error(1,"db open failed for '%s'",fn);
	v=MARKER;  gzwrite(fd,&v,sizeof(unsigned int));
	v=VERSION; gzwrite(fd,&v,sizeof(unsigned int));
	gzwrite(fd,&dt->t,sizeof(time_t));
	for(ch=0;ch<HNCH;ch++) for(df=dt->fhsh[ch];df;df=df->nxt){
		gzprintf(fd,"%s\n",df->fn);
		gzwrite(fd,&df->st,sizeof(struct st));
		switch(df->st.typ){
		case FT_FILE: gzwrite(fd,dbhgetsha(df->dh),sizeof(unsigned char)*SHALEN); break;
		case FT_LNK: gzwrite(fd,df->lnk,sizeof(char)*FNLEN); break;
		case FT_DIR: break;
		case FT_NONE: break;
		}
	}
	gzprintf(fd,"\n");
	gzclose(fd);
}

const char *dbbdir(){ return db.bdir; }
struct ex *dbgetex(){ return db.ex; }

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
	/* TODO set df->c + df->cnxt */
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
void dbfseth(struct dbf *df,struct dbh *dh){ df->dh=dh; }
char *dbfgetlnk(struct dbf *df){ return df->lnk; }

