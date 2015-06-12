#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd.h"
#include "main.h"
#include "dbt.h"
#include "help.h"
#include "sha.h"
#include "dirrec.h"
#include "dbhwrk.h"
#include "fq.h"
#include "fnsort.h"
#include "ext2.h"

void init(const char *basedir,const char *exclude){
	FILE *fd;
	printf("[init %s",basedir);
	if(exclude) printf(" (%s)",exclude);
	printf("]\n");
	mkd(DD "/");
	mkd(DH "/");
	if(!(fd=fopen("basedir","w"))) error(1,"basedir open failed");
	fprintf(fd,"%s\n",basedir);
	fclose(fd);
	if(!exclude) return;
	if(!(fd=fopen("exclude","w"))) error(1,"exclude open failed");
	fprintf(fd,"%s\n",exclude);
	fclose(fd);
}

void tlistt(struct dbt *dt){
	int nf=0;
	struct dbf *df=NULL;
	struct dbh *dh=NULL;
	size_t si=0,ex=0,gz=0,sum=0;
	while((df=dbfgetnxt(dt,df))){ nf++; si+=dbfgetst(df)->size; }
	while((dh=dbhgetnxt(dh))){
		enum dbhex r=dbhexdt(dh,dt);
		if(r&DE_IN) gz+=dbhgetsi(dh);
		if(!(r&DE_EX)) ex+=dbhgetsi(dh);
		if(!(r&DE_OLD)) sum+=dbhgetsi(dh);
	}
	printf("%s nf %4i si %5s",timefmt(dbtgett(dt)),nf,sizefmt(si));
	printf(" gz %5s",sizefmt(gz));
	printf(" ex %5s",sizefmt(ex));
	printf(" sum %5s",sizefmt(sum));
	printf("\n");
}

void tlist(){
	struct dbt *dt=NULL;
	printf("[tlist]\n");
	while((dt=dbtgetnxt(dt))) tlistt(dt);
}

void flistt(struct dbt *dt,char ext2){
	struct dbf *df=NULL;
	struct fns *fns=fnsinit();
	const char *fn;
	printf("[flist %s]\n",timefmt(dbtgett(dt)));
	while((df=dbfgetnxt(dt,df))) fnsadd(fns,dbfgetfn(df),0,df);
	while((fn=fnsnxt(fns,(void**)&df))){
		const unsigned char *sha=dbhgetsha(dbfgeth(df));
		int i;
		struct st *st=dbfgetst(df);
		size_t si=dbfgetsi(df);
		printf("  ");
		if(sha) for(i=0;i<4;i++) printf("%02x",sha[i]);
		else printf("%8s","");
		printf(" %5s",sizefmt(st->size));
		printf(" %5s",si?sizefmt(si):"");
		printf(" %s\n",fn);
		if(st->typ==FT_EXT2 && ext2) ext2list(dbfgetext2(df));
	}
}

void flist(const char *stime,char ext2){
	if(stime) flistt(timeparse(stime),ext2);
	else{
		struct dbt *dt=NULL;
		while((dt=dbtgetnxt(dt))) flistt(dt,ext2);
	}
}

struct dbt *timenewest(const char *stime){
	struct dbt *dt;
	if(!stime || !stime[0]){ if(!(dt=dbtgetnewest())) error(1,"no time found in db"); }
	else if(!(dt=timeparse(stime))) error(1,"unkown time: '%s'",stime);
	return dt;
}

struct darg {
	struct dbt *dt;
	struct fns *fns;
};

int difile(const char *fn,enum ftyp typ,void *arg){
	struct darg *da=(struct darg*)arg;
	struct dbf *df;
	struct st st;
	enum statcmp sd;
	if(!(df=dbfget(da->dt,fn))){ fnsadd(da->fns,fn,1,(void*)SD_NEW); return 1; }
	statget(1,fn,&st);
	*dbfgetmk(df)=1;
	if((sd=statcmp(&st,dbfgetst(df)))){ fnsadd(da->fns,dbfgetfn(df),0,(void*)sd); return 1; }
	return 0;
}

int difilesha(const char *fn,enum ftyp typ,void *arg){
	struct darg *da=(struct darg*)arg;
	struct dbf *df;
	unsigned char sha[SHALEN];
	struct dbh *dh;
	if(difile(fn,typ,arg)) return 1;
	if(!(df=dbfget(da->dt,fn))){ fnsadd(da->fns,fn,1,(void*)SD_NEW); return 1; }
	/* TODO: FT_EXT2 -> loop over blocks */
	if(!(dh=dbfgeth(df))) return 0;
	shaget(fn,sha);
	if(memcmp(sha,dbhgetsha(dh),SHALEN)){ fnsadd(da->fns,dbfgetfn(df),0,(void*)SD_SHA); return 1; }
	return 0;
}

char difft(struct dbt *dt,char sha){
	struct dbf *df;
	int chg;
	struct darg da={.dt=dt};
	const char *fn;
	void *sd;
	da.fns=fnsinit();
	printf("[diff %s]\n",timefmt(dbtgett(dt)));
	for(df=NULL;(df=dbfgetnxt(dt,df));) *dbfgetmk(df)=0;
	chg=dirrec(dbbdir(),dbgetex(),"",sha?difilesha:difile,&da);
	for(df=NULL;(df=dbfgetnxt(dt,df));) if(!*dbfgetmk(df)){ fnsadd(da.fns,dbfgetfn(df),0,(void*)SD_DEL); chg++; }
	while((fn=fnsnxt(da.fns,&sd))) printf("%-6s: %s\n",statcmpfmt((int)((unsigned long)sd)),fn);
	return chg!=0;
}

void diff(const char *stime,char sha){ difft(timenewest(stime),sha); }

char cifilesha(struct dbt *dt,struct dbf *df){
	unsigned char sha[SHALEN];
	struct dbh *dh;
	char ret=0;
	if(ext2read(df,dt)) return 0;
	shaget(dbfgetfn(df),sha);
	if(!(dh=dbhget(sha))){ ret=1; dh=dbhnew(sha,0); }
	dbhadd(dh,dt,df);
	return ret;
}

void cifilecp(struct dbf *df){
	struct dbh *dh=dbfgeth(df);
	dbhsetsi(dh,dbhsave(dbhgetsha(dh),dbfgetfn(df)));
}

int cifile(const char *fn,enum ftyp typ,void *vdt){
	struct dbt *dtn=((struct dbt**)vdt)[0];
	struct dbt *dt =((struct dbt**)vdt)[1];
	struct dbf *df=dbfnew(dt,fn);
	struct dbf *dfn=dtn?dbfget(dtn,fn):NULL;
	struct st *st;
	statget(1,fn,st=dbfgetst(df));
	switch(st->typ){
	case FT_FILE:
		if(dfn && !statcmp(st,dbfgetst(dfn))){
			struct dbh *dh=dbfgeth(dfn);
			if(dh) dbhadd(dh,dt,df);
			else if(dbfgetst(dfn)->typ==FT_EXT2) dbfsetext2(df,dbfgetext2(dfn)); /* TODO: set dh->hf */
		}else fqadd(df);
	break;
	case FT_DIR: break;
	case FT_LNK: lnkget(fn,dbfgetlnk(df)); break;
	case FT_NONE: error(0,"no backup for none regular file: '%s'",fn);
	case FT_EXT2: break;
	}
	return 1;
}

void commit(){
	struct dbt *dt[2];
	dt[0]=dbtgetnewest();
	if(dt[0] && !difft(dt[0],0)) error(1,"no changes -> no commit");
	dt[1]=dbtnew(0);
	printf("[commit %s]\n",timefmt(dbtgett(dt[1])));
	printf("[copy %i files]\n",dirrec(dbbdir(),dbgetex(),"",cifile,dt));
	fqrun(dt[1],cifilesha,cifilecp);
	dbtsave(dt[1]);
	tlistt(dt[1]);
}

void restoref(struct dbf *df,const char *dstdir){
	struct st *st;
	char fn[FNLEN];
	struct dbf *dfc;
	st=dbfgetst(df);
	snprintf(fn,FNLEN,"%s%s%s",dstdir,dbfgetfn(df),st->typ==FT_DIR?"/":"");
	switch(st->typ){
	case FT_FILE: dbhrestore(dbhgetsha(dbfgeth(df)),fn); break; /* TODO: sort by file pos */
	case FT_DIR: mkd(fn); break;
	case FT_LNK: mkd(fn); lnkset(dbfgetlnk(df),fn); break;
	case FT_NONE: error(0,"no restore for none regular file: '%s'",fn); break;
	case FT_EXT2: ext2restore(df,fn); break;
	}
	for(dfc=dbfgetc(df);dfc;dfc=dbfgetcnxt(dfc)) restoref(dfc,dstdir);
	statset(st,fn);
}

void restore(const char *fn,const char *stime,const char *dstdir){
	struct dbt *dt=timenewest(stime);
	struct dbf *df;
	if(!dstdir) dstdir="restore";
	printf("[restore %s (%s -> %s)]\n",timefmt(dbtgett(dt)),fn&&fn[0]?fn:"/",dstdir);
	dbtsetc(dt);
	if(!fn || !fn[0]) for(df=dbtgetc(dt);df;df=dbfgetcnxt(df)) restoref(df,dstdir);
	else{
		if(!(df=dbfget(dt,fn))) error(1,"file not found: '%s'\n",fn);
		restoref(df,dstdir);

	}
}

void del(const char *stime){
	struct dbt *dt=timeparse(stime);
	struct dbf *df=NULL;
	printf("[del %s]\n",timefmt(dbtgett(dt)));
	tlistt(dt);
	while((df=dbfgetnxt(dt,df))){
		struct dbh *dh=dbfgeth(df);
		/* TODO: sort by inode */
		if(dbfgetst(df)->typ==FT_FILE && dbhexdt(dh,dt)) dbhdel(dbhgetsha(dh));
	}
	dbtdel(dt);
}

void dbcheck(){
	struct dbh *dh;
	struct dbt *dt;
	struct dbf *df;
	printf("[dbcheck]\n");
	for(dt=NULL;(dt=dbtgetnxt(dt));) for(df=NULL;(df=dbfgetnxt(dt,df));) if((dh=dbfgeth(df))) *dbhgetmk(dh)=1;
	for(dh=NULL;(dh=dbhgetnxt(dh));){
		char fn[FNLEN];
		sha2fn(dbhgetsha(dh),fn);
		if(*dbhgetmk(dh)){
			unsigned char sha[SHALEN];
			/* TODO: sort by file pos */
			shagetdb(fn,sha);
			if(memcmp(sha,dbhgetsha(dh),SHALEN)) error(0,"sha missmatch: '%s'",fn);
		}else error(0,"unused file: '%s'",fn);
	}
}

