#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <zlib.h>

#include "dbt.h"
#include "main.h"
#include "help.h"
#include "sha.h"
#include "dbhwrk.h"
#include "ex.h"

#define HNCH	8192

#define VERSION	1
#define MARKER	0x4381E32F

struct dbf {
	struct dbf *nxt;
	char fn[FNLEN];
	union {
		char lnk[FNLEN];
		struct dbe *de;
	} u;
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
	struct dbf *c;
};

struct db {
	char bdir[FNLEN];
	struct ex *ex;
	struct dbt *dt;
} db = {
	.dt=NULL,
	.ex=NULL,
};

unsigned int fkey(const char *fn){
	unsigned char sha[SHALEN];
	unsigned int *fk=(unsigned int*)sha;
	shastr(fn,sha);
	return fk[0]%HNCH;
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
					error(0,"dbh file missing: '%s'",fn);
				}else dbhadd(dh,dt,df);
			} break;
			case FT_LNK: gzread(gd,df->u.lnk,sizeof(char)*FNLEN); break;
			case FT_DIR: break;
			case FT_NONE: break;
			case FT_EXT2: dbfsetext2(df,ext2load(&gd,dt,df)); break;
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
		case FT_LNK: gzwrite(fd,df->u.lnk,sizeof(char)*FNLEN); break;
		case FT_EXT2: ext2save(dbfgetext2(df),&fd); break;
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
	struct dbt *dt=calloc(1,sizeof(struct dbt)), **di=&db.dt;
	dt->t= t ? t : time(NULL);
	while(di[0] && di[0]->t<t) di=&di[0]->nxt;
	dt->nxt=di[0];
	di[0]=dt;
	return dt;
}

struct dbt *dbtget(time_t t){
	struct dbt *dt=db.dt;
	while(dt && dt->t!=t) dt=dt->nxt;
	return dt;
}

struct dbt *dbtgetnxt(struct dbt *dt){ return dt ? dt->nxt : db.dt; }

struct dbt *dbtgetnewest(){
	struct dbt *dt=db.dt;
	while(dt && dt->nxt) dt=dt->nxt;
	return dt;
}

time_t dbtgett(struct dbt *dt){ return dt->t; }
struct dbf *dbtgetc(struct dbt *dt){ return dt->c; }

void dbtsetc(struct dbt *dt){
	struct dbf *df=NULL;
	char *p;
	while((df=dbfgetnxt(dt,df))) if((p=strrchr(df->fn,'/')) && p>df->fn){
		struct dbf *dfp;
		*p='\0';
		if((dfp=dbfget(dt,df->fn))){
			df->cnxt=dfp->c;
			dfp->c=df;
		}else error(0,"parent not found: '%s'",df->fn);
		*p='/';
	}else{
		df->cnxt=dt->c;
		dt->c=df;
	}
}

void dbtdel(struct dbt *dt){
	char fn[FNLEN];
	snprintf(fn,FNLEN,DD "/%li.dbt",dt->t);
	unlink(fn);
}

struct dbf *dbfnew(struct dbt *dt,const char *fn){
	struct dbf *df=calloc(1,sizeof(struct dbf));
	unsigned int fk=fkey(fn);
	df->nxt=dt->fhsh[fk];
	dt->fhsh[fk]=df;
	memcpy(df->fn,fn,FNLEN);
	return df;
}

struct dbf *dbfget(struct dbt *dt,const char *fn){
	struct dbf *df=dt->fhsh[fkey(fn)];
	while(df && strncmp(fn,df->fn,FNLEN)) df=df->nxt;
	return df;
}

struct dbf *dbfgetnxt(struct dbt *dt,struct dbf *df){
	unsigned int fk;
	if(df && df->nxt) return df->nxt;
	fk = df ? fkey(df->fn)+1 : 0;
	for(;fk<HNCH;fk++) if(dt->fhsh[fk]) return dt->fhsh[fk];
	return NULL;
}

void dbfsetext2(struct dbf *df,struct dbe *de){
	df->st.typ=FT_EXT2;
	df->u.de=de;
	df->dh=NULL;
}

struct dbe *dbfgetext2(struct dbf *df){ return df->u.de; }
const char *dbfgetfn(struct dbf *df){ return df->fn; }
struct st *dbfgetst(struct dbf *df){ return &df->st; }
char *dbfgetmk(struct dbf *df){ return &df->mk; }
struct dbh *dbfgeth(struct dbf *df){ return df->dh; }
void dbfseth(struct dbf *df,struct dbh *dh){ df->dh=dh; }
char *dbfgetlnk(struct dbf *df){ return df->u.lnk; }
struct dbf *dbfgetc(struct dbf *df){ return df->c; }
struct dbf *dbfgetcnxt(struct dbf *df){ return df->cnxt; }

size_t dbfgetsi(struct dbf *df){
	if(df->dh) return dbhgetsi(df->dh);
	if(df->st.typ==FT_EXT2){
		size_t si=0;
		struct dbh *dh=NULL;
		while((dh=dbhgetnxt(dh))) if(dbhexdf(dh,df)&DE_IN) si+=dbhgetsi(dh);
		return si;
	}
	return 0;
}

