#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "db.h"
#include "main.h"
#include "help.h"

#define HNCH	8192

#define VERSION	1
#define MARKER	0x4381E32F

struct dbf {
	struct dbf *nxt;
	char fn[FNLEN];
	struct mstat st;
	unsigned char sha[SHALEN];
	char mk;
};

struct dbt {
	struct dbt *nxt;
	time_t t;
	struct dbf *fhsh[HNCH];
};

struct db {
	char bdir[FNLEN];
	struct dbt *dt;
} db = {
	.dt=NULL,
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
		if(!(fd=fopen(fn,"rb"))) error(1,"dbt file not readable: '%s'",fn);
		fread(&v,sizeof(unsigned int),1,fd); if(v!=MARKER)  error(1,"dbt file with wrong marker: '%s'",fn);
		fread(&v,sizeof(unsigned int),1,fd); if(v!=VERSION) error(1,"dbt file with wrong version: '%s'",fn);
		fread(&t,sizeof(time_t),1,fd);
		dt=dbtnew(t);
		while(fgets(ffn,FNLEN,fd) && ffn[0]!='\n'){
			struct dbf *df=dbfnew(dt,fnrmnewline(ffn));
			fread(&df->st,sizeof(struct mstat),1,fd);
			fread(df->sha,sizeof(unsigned char),SHALEN,fd);
		}
		fclose(fd);
	}
	closedir(dd);
}

void dbtsave(struct dbt *dt){
	char fn[FNLEN];
	FILE *fd;
	unsigned int v;
	int ch;
	struct dbf *df;
	snprintf(fn,FNLEN,DD "/%li.dbt",dt->t);
	if(!(fd=fopen(fn,"wb"))) error(1,"db open failed for '%s'",fn);
	v=MARKER;  fwrite(&v,sizeof(unsigned int),1,fd);
	v=VERSION; fwrite(&v,sizeof(unsigned int),1,fd);
	fwrite(&dt->t,sizeof(time_t),1,fd);
	for(ch=0;ch<HNCH;ch++) for(df=dt->fhsh[ch];df;df=df->nxt){
		fprintf(fd,"%s\n",df->fn);
		fwrite(&df->st,sizeof(struct mstat),1,fd);
		fwrite(df->sha,sizeof(unsigned char),SHALEN,fd);
	}
	fprintf(fd,"\n");
	fclose(fd);
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
struct mstat *dbfgetst(struct dbf *df){ return &df->st; }
unsigned char *dbfgetsha(struct dbf *df){ return df->sha; }
char *dbfgetmk(struct dbf *df){ return &df->mk; }

