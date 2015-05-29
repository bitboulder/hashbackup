#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd.h"
#include "main.h"
#include "db.h"
#include "help.h"
#include "dat.h"

void init(const char *basedir){
	FILE *fd;
	printf("[init]\n");
	mkd(DD "/");
	mkd(DH "/");
	if(!(fd=fopen("basedir","w"))) error(1,"basedir open failed");
	fprintf(fd,"%s\n",basedir);
	fclose(fd);
}

void tlistt(struct dbt *dt){
	int nf=0;
	struct dbf *df=NULL;
	size_t si=0,ex=0,gz=0;
	while((df=dbfgetnxt(dt,df))){
		struct st *st=dbfgetst(df);
		struct dbh *dh=dbfgeth(df);
		si+=st->size;
		if(dh){
			gz+=dbhgetsi(dh);
			if(dbhexdt(dh,dt)) ex+=dbhgetsi(dh);
		}
		nf++;
	}
	printf("%s nf %4i si %5s ",timefmt(dbtgett(dt)),nf,sizefmt(si));
	printf("gz %5s ",sizefmt(gz));
	printf("ex %5s\n",sizefmt(ex));
}

void tlist(){
	struct dbt *dt=NULL;
	printf("[tlist]\n");
	while((dt=dbtgetnxt(dt))) tlistt(dt);
}

void flistt(struct dbt *dt){
	struct dbf *df=NULL;
	printf("t %lu\n",dbtgett(dt));
	while((df=dbfgetnxt(dt,df))){
		int i;
		unsigned char *sha=dbhgetsha(dbfgeth(df));
		printf("  ");
		for(i=0;i<SHALEN;i++) printf("%02x",sha[i]);
		printf(" %s\n",dbfgetfn(df));
	}
}

void flist(const char *stime){
	printf("[flist %s]\n",stime);
	if(stime) flistt(timeparse(stime));
	else{
		struct dbt *dt=NULL;
		while((dt=dbtgetnxt(dt))) flistt(dt);
	}
}

int difile(const char *fn,void *vdt){
	struct dbt *dt=(struct dbt*)vdt;
	struct dbf *df;
	struct st st;
	if(!(df=dbfget(dt,fn))){ printf("new: %s\n",fn); return 1; }
	statget(1,fn,&st);
	*dbfgetmk(df)=1;
	if(memcmp(&st,dbfgetst(df),sizeof(struct st))){ printf("mod: %s\n",fn); return 1; }
	return 0;
}

struct dbt *timenewest(const char *stime){
	struct dbt *dt;
	if(!stime){ if(!(dt=dbtgetnewest())) error(1,"no time found in db"); }
	else if(!(dt=timeparse(stime))) error(1,"unkown time: '%s'",stime);
	return dt;
}

char difft(struct dbt *dt){
	struct dbf *df;
	int chg;
	printf("[diff %s]\n",timefmt(dbtgett(dt)));
	for(df=NULL;(df=dbfgetnxt(dt,df));) *dbfgetmk(df)=0;
	chg=dirrec(dbbdir(),"",difile,dt);
	for(df=NULL;(df=dbfgetnxt(dt,df));) if(!*dbfgetmk(df)){ printf("del: %s\n",dbfgetfn(df)); chg++; }
	return chg!=0;
}

void diff(const char *stime){ difft(timenewest(stime)); }

int cifile(const char *fn,void *vdt){
	struct dbt *dtn=((struct dbt**)vdt)[0];
	struct dbt *dt =((struct dbt**)vdt)[1];
	struct dbf *df=dbfnew(dt,fn);
	struct dbf *dfn=dtn?dbfget(dtn,fn):NULL;
	struct st *st;
	statget(1,fn,st=dbfgetst(df));
	switch(st->mode){
	case MS_FILE:
		if(dfn && !memcmp(st,dbfgetst(dfn),sizeof(struct st))){
			struct dbh *dh=dbfgeth(dfn);
			if(dh) dbhadd(dh,dt,df);
		}else{
			unsigned char sha[SHALEN];
			shaget(fn,sha);
			datadd(sha,fn);
			dbhadd(dbhget(sha),dt,df);
		}
	break;
	case MS_DIR: break;
	case MS_LNK: lnkget(fn,dbfgetlnk(df)); break;
	case MS_NONE: error(0,"no backup for none regular file: '%s'",fn);
	}
	return 0;
}

void commit(){
	struct dbt *dt[2];
	dt[0]=dbtgetnewest();
	if(dt[0] && !difft(dt[0])) error(1,"no changes -> no commit");
	dt[1]=dbtnew(0);
	printf("[commit %s]\n",timefmt(dbtgett(dt[1])));
	dirrec(dbbdir(),"",cifile,dt);
	dbtsave(dt[1]);
	tlistt(dt[1]);
}

void restore(const char *dstdir,const char *stime){
	struct dbt *dt=timenewest(stime);
	struct dbf *df=NULL;
	printf("[restore %s -> %s]\n",timefmt(dbtgett(dt)),dstdir);
	while((df=dbfgetnxt(dt,df))){
		struct st *st=dbfgetst(df);
		char fn[FNLEN];
		snprintf(fn,FNLEN,"%s/%s%s",dstdir,dbfgetfn(df),st->mode==MS_DIR?"/":"");
		switch(st->mode){
		case MS_FILE: datget(dbhgetsha(dbfgeth(df)),fn); break;
		case MS_DIR: mkd(fn); break;
		case MS_LNK: mkd(fn); lnkset(dbfgetlnk(df),fn); break;
		case MS_NONE: error(0,"no restore for none regular file: '%s'",fn); break;
		}
		statset(st,fn);
	}
}

void del(const char *stime){
	struct dbt *dt=timeparse(stime);
	struct dbf *df=NULL;
	printf("[del %s]\n",timefmt(dbtgett(dt)));
	tlistt(dt);
	while((df=dbfgetnxt(dt,df))){
		struct dbh *dh=dbfgeth(df);
		if(dbfgetst(df)->mode==MS_FILE && dbhexdt(dh,dt)) datdel(dbhgetsha(dh));
	}
	dbtdel(dt);
}

void dbcheck(){
	printf("[dbcheck]\n");
	
}

