#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cmd.h"
#include "main.h"
#include "db.h"
#include "help.h"

void init(const char *basedir){
	FILE *fd;
	system("mkdir " DD " " DH);
	if(!(fd=fopen("basedir","w"))) error(1,"basedir open failed");
	fprintf(fd,"%s\n",basedir);
	fclose(fd);
}

void diff(const char *stime){
}

void cifile(const char *fn,void *vdt){
	struct dbt *dt=(struct dbt*)vdt;
	struct dbf *df=dbfnew(dt,fn);
	struct stat *st;
	if((st=mstat(fn))) *dbfgetst(df)=*st;
}

void commit(){
	struct dbt *dt=dbtnew(0);
	dirrec(dbbdir(),"",cifile,dt);
	dbtsave(dt);
}

void dbcheck(){
}

void tlist(){
	struct dbt *dt=NULL;
	while((dt=dbtgetnxt(dt))){
		int n=0;
		struct dbf *df=NULL;
		while((df=dbfgetnxt(dt,df))) n++;
		printf("t %lu n %i\n",dbtgett(dt),n);
	}
}

void flistt(struct dbt *dt){
	struct dbf *df=NULL;
	printf("t %lu\n",dbtgett(dt));
	while((df=dbfgetnxt(dt,df))) printf("  %s\n",dbfgetfn(df));
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

