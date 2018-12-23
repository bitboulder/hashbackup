#define _DEFAULT_SOURCE
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
	struct str fn;
	union {
		struct str lnk;
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
	struct str bdir;
	struct ex *ex;
	struct dbt *dt;
} db = {
	.bdir=STRDEF,
	.dt=NULL,
	.ex=NULL,
};

unsigned int fkey(const char *fn){
	unsigned char sha[SHALEN];
	unsigned int *fk=(unsigned int*)&sha[0];
	shastr(fn,sha);
	return fk[0]%HNCH;
}

char dbloadbdir(){
	FILE *fd;
	if(!(fd=fopen("basedir","r"))) return 0;
	str_setlen(&db.bdir,FNFLEN);
	if(!fgets(db.bdir.s,db.bdir.l,fd)) return 0;
	fclose(fd);
	db.bdir.s[db.bdir.l-1]='\0';
	fnrmnewline(&db.bdir);
	str_clip(&db.bdir);
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
		char fn[FNFLEN];
		int i=0;
		unsigned int v;
		time_t t;
		struct dbt *dt;
		struct str ffn=STRDEF;
		struct str tmp=STRDEF;
		str_setlen(&ffn,FNFLEN);
		str_setlen(&tmp,FNFLEN);
		while(i<251 && di->d_name[i]>='0' && di->d_name[i]<='9') i++;
		if(strncmp(di->d_name+i,".dbt",4)) continue;
		di->d_name[i+4]='\0';
		snprintf(fn,sizeof(fn),DD "/%s",di->d_name);
		if(!(gd=gzopen(fn,"rb"))) error(1,"dbt file not readable: '%s'",fn);
		gzread(gd,&v,sizeof(unsigned int)); if(v!=MARKER)  error(1,"dbt file with wrong marker: '%s'",fn);
		gzread(gd,&v,sizeof(unsigned int)); if(v!=VERSION) error(1,"dbt file with wrong version: '%s'",fn);
		gzread(gd,&t,sizeof(time_t));
		dt=dbtnew(t);
		while(gzgets(gd,ffn.s,ffn.l) && ffn.s[0]!='\n' && ffn.s[0]){
			struct dbf *df=dbfnew(dt,*fnrmnewline(&ffn));
			gzread(gd,&df->st,sizeof(struct st));
			switch(df->st.typ){
			case FT_FILE: {
				unsigned char sha[SHALEN];
				struct dbh *dh;
				gzread(gd,sha,sizeof(unsigned char)*SHALEN);
				dh=dbhget(sha);
				if(!dh){
					sha2fn(sha,&tmp);
					error(0,"dbh file missing: '%s'",tmp.s);
				}else dbhadd(dh,dt,df);
			} break;
			case FT_LNK: gzread(gd,tmp.s,tmp.l); str_copy(&df->u.lnk,tmp,1); break;
			case FT_DIR: break;
			case FT_NONE: break;
			case FT_EXT2: dbfsetext2(df,ext2load(&gd,dt,df)); break;
			}
		}
		gzclose(gd);
		str_setlen(&ffn,0);
		str_setlen(&tmp,0);
	}
	closedir(dd);
}

void dbtsave(struct dbt *dt){
	char fn[FNFLEN];
	gzFile fd;
	unsigned int v;
	int ch;
	struct dbf *df;
	struct str tmp=STRDEF;
	str_setlen(&tmp,FNFLEN);
	printf("[dbtsave]\n");
	snprintf(fn,sizeof(fn),DD "/%li.dbt",dt->t);
	if(!(fd=gzopen(fn,"wb"))) error(1,"db open failed for '%s'",fn);
	v=MARKER;  gzwrite(fd,&v,sizeof(unsigned int));
	v=VERSION; gzwrite(fd,&v,sizeof(unsigned int));
	gzwrite(fd,&dt->t,sizeof(time_t));
	for(ch=0;ch<HNCH;ch++) for(df=dt->fhsh[ch];df;df=df->nxt){
		gzprintf(fd,"%s\n",df->fn.s);
		gzwrite(fd,&df->st,sizeof(struct st));
		switch(df->st.typ){
		case FT_FILE: gzwrite(fd,dbhgetsha(df->dh),sizeof(unsigned char)*SHALEN); break;
		case FT_LNK: str_copy(&tmp,df->u.lnk,0); gzwrite(fd,tmp.s,tmp.l); break;
		case FT_EXT2: ext2save(dbfgetext2(df),&fd); break;
		case FT_DIR: break;
		case FT_NONE: break;
		}
	}
	gzprintf(fd,"\n");
	gzclose(fd);
	str_setlen(&tmp,0);
}

const char *dbbdir(){ return db.bdir.s; }
struct ex *dbgetex(){ return db.ex; }

struct dbt *dbtnew(time_t t){
	struct dbt *dt=calloc(1,sizeof(struct dbt)), **di=&db.dt;
	if(!t) t=time(NULL);
	while(di[0] && di[0]->t<=t){
		if(di[0] && di[0]->t==t) t++;
		di=&di[0]->nxt;
	}
	dt->t=t;
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
	while((df=dbfgetnxt(dt,df))) if((p=strrchr(df->fn.s,'/')) && p>df->fn.s){
		struct dbf *dfp;
		*p='\0';
		if((dfp=dbfget(dt,df->fn.s))){
			df->cnxt=dfp->c;
			dfp->c=df;
		}else error(0,"parent not found: '%s'",df->fn.s);
		*p='/';
	}else{
		df->cnxt=dt->c;
		dt->c=df;
	}
}

void dbtdel(struct dbt *dt){
	char fn[FNFLEN];
	snprintf(fn,sizeof(fn),DD "/%li.dbt",dt->t);
	unlink(fn);
}

struct dbf *dbfnew(struct dbt *dt,const struct str fn){
	struct dbf *df=calloc(1,sizeof(struct dbf));
	unsigned int fk=fkey(fn.s);
	df->nxt=dt->fhsh[fk];
	dt->fhsh[fk]=df;
	str_copy(&df->fn,fn,1);
	return df;
}

struct dbf *dbfget(struct dbt *dt,const char *fn){
	struct dbf *df=dt->fhsh[fkey(fn)];
	while(df && strncmp(fn,df->fn.s,MIN(strlen(fn),df->fn.l))) df=df->nxt;
	return df;
}

struct dbf *dbfgetnxt(struct dbt *dt,struct dbf *df){
	unsigned int fk;
	if(df && df->nxt) return df->nxt;
	fk = df ? fkey(df->fn.s)+1 : 0;
	for(;fk<HNCH;fk++) if(dt->fhsh[fk]) return dt->fhsh[fk];
	return NULL;
}

void dbfsetext2(struct dbf *df,struct dbe *de){
	df->st.typ=FT_EXT2;
	df->u.de=de;
	df->dh=NULL;
}

struct dbe *dbfgetext2(struct dbf *df){ return df->u.de; }
const char *dbfgetfn(struct dbf *df){ return df->fn.s; }
struct st *dbfgetst(struct dbf *df){ return &df->st; }
char *dbfgetmk(struct dbf *df){ return &df->mk; }
struct dbh *dbfgeth(struct dbf *df){ return df->dh; }
void dbfseth(struct dbf *df,struct dbh *dh){ df->dh=dh; }
struct str *dbfgetlnk(struct dbf *df){ return &df->u.lnk; }
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

