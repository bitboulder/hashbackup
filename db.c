#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "db.h"
#include "main.h"

#define HNCH	8192

struct dbf {
	struct dbf *nxt;
	char fn[FNLEN];
};

struct dbt {
	struct dbt *nxt;
	time_t t;
	struct dbf *fhsh[HNCH];
};

struct dbt *db=NULL;

int fkey(const char *fn){
	return 0;
}

char *fnrmnewline(char *fn){
	int i;
	for(i=0;i<FNLEN-1 && fn[i] && fn[i]!='\n';) i++;
	fn[i]='\0';
	return fn;
}

void dbload(const char *fn){
	FILE *fd;
	time_t t;
	int v;
	if(!(fd=fopen(fn,"rb"))) return;
	fread(&v,sizeof(int),1,fd);
	while(fread(&t,sizeof(time_t),1,fd)==1 && t){
		struct dbt *dt=dbtnew(t);
		char ffn[FNLEN];
		while(fgets(ffn,FNLEN,fd) && ffn[0]!='\n'){
			struct dbf *df=dbfnew(dt,fnrmnewline(ffn));
		}
	}
	fclose(fd);
}

void dbsave(const char *fn){
	FILE *fd;
	struct dbt *dt;
	int v=1;
	time_t t=0;
	if(!(fd=fopen(fn,"wb"))) error("db open failed for '%s'",fn);
	fwrite(&v,sizeof(int),1,fd);
	for(dt=db;dt;dt=dt->nxt){
		int ch;
		struct dbf *df;
		fwrite(&dt->t,sizeof(time_t),1,fd);
		for(ch=0;ch<HNCH;ch++) for(df=dt->fhsh[ch];df;df=df->nxt){
			fprintf(fd,"%s\n",df->fn);
		}
		fprintf(fd,"\n");
	}
	fwrite(&t,sizeof(time_t),1,fd);
	fclose(fd);
}

struct dbt *dbtnew(time_t t){
	struct dbt *dt=calloc(1,sizeof(struct dbt));
	dt->t= t ? t : time(NULL);
	dt->nxt=db;
	db=dt;
	return dt;
}

struct dbt *dbtgetnxt(struct dbt *dt){ return dt ? dt->nxt : db; }
time_t dbtgett(struct dbt *dt){ return dt->t; }

struct dbf *dbfnew(struct dbt *dt,const char *fn){
	struct dbf *df=calloc(1,sizeof(struct dbf));
	int fk=fkey(fn);
	df->nxt=dt->fhsh[fk];
	dt->fhsh[fk]=df;
	memcpy(df->fn,fn,FNLEN);
	return df;
}

struct dbf *dbfgetnxt(struct dbt *dt,struct dbf *df){
	int ch;
	if(df && df->nxt) return df->nxt;
	ch = df ? fkey(df->fn)+1 : 0;
	for(;ch<HNCH;ch++) if(dt->fhsh[ch]) return dt->fhsh[ch];
	return NULL;
}

