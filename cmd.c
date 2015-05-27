#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cmd.h"
#include "main.h"
#include "db.h"
#include "help.h"
#include "dat.h"

void init(const char *basedir){
	FILE *fd;
	system("mkdir " DD " " DH);
	if(!(fd=fopen("basedir","w"))) error(1,"basedir open failed");
	fprintf(fd,"%s\n",basedir);
	fclose(fd);
}

void difile(const char *fn,void *vdt){
	struct dbt *dt=(struct dbt*)vdt;
	struct dbf *df;
	struct mstat st;
	if(!(df=dbfget(dt,fn))){ printf("new: %s\n",fn); return; }
	mstat(fn,&st);
	if(memcmp(&st,dbfgetst(df),sizeof(struct mstat))) printf("mod: %s\n",fn);
	*dbfgetmk(df)=1;
}

void diff(const char *stime){
	struct dbt *dt;
	struct dbf *df;
	if(!stime){ if(!(dt=dbtgetnewest())) error(1,"no time found in db"); }
	else if(!(dt=timeparse(stime))) error(1,"unkown time: '%s'",stime);
	for(df=NULL;(df=dbfgetnxt(dt,df));) *dbfgetmk(df)=0;
	dirrec(dbbdir(),"",difile,dt);
	for(df=NULL;(df=dbfgetnxt(dt,df));) if(!*dbfgetmk(df)) printf("del: %s\n",dbfgetfn(df));
}

void cifile(const char *fn,void *vdt){
	struct dbt *dtn=((struct dbt**)vdt)[0];
	struct dbt *dt =((struct dbt**)vdt)[1];
	struct dbf *df=dbfnew(dt,fn);
	struct dbf *dfn=dtn?dbfget(dtn,fn):NULL;
	struct mstat *st;
	mstat(fn,st=dbfgetst(df));
	if(dfn && !memcmp(st,dbfgetst(dfn),sizeof(struct mstat))){
		struct dbh *dh=dbfgeth(dfn);
		if(dh) dbhadd(dh,dt,df);
		return;
	}
	switch(st->mode){
	case MS_FILE: {
		unsigned char sha[SHALEN];
		msha(fn,sha);
		dbhadd(dbhget(sha),dt,df);
		datadd(sha,fn);
	} break;
	/* TODO others */
	}
}

void commit(){
	struct dbt *dt[2];
	dt[0]=dbtgetnewest();
	dt[1]=dbtnew(0);
	dirrec(dbbdir(),"",cifile,dt);
	dbtsave(dt[1]);
}

void dbcheck(){
}

void tlist(){
	struct dbt *dt=NULL;
	while((dt=dbtgetnxt(dt))){
		int nf=0;
		struct dbf *df=NULL;
		size_t si=0,ex=0;
		while((df=dbfgetnxt(dt,df))){
			struct mstat *st=dbfgetst(df);
			si+=st->size;
			if(dbhexdt(dbfgeth(df),dt)) ex+=st->size;
			nf++;
		}
		printf("%s nf %4i si %5s /",timefmt(dbtgett(dt)),nf,sizefmt(ex));
		printf("%5s\n",sizefmt(si));
	}
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
	if(stime) flistt(timeparse(stime));
	else{
		struct dbt *dt=NULL;
		while((dt=dbtgetnxt(dt))) flistt(dt);
	}
}

void del(const char *stime){
}

