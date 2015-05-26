#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "main.h"
#include "db.h"
#include "add.h"

void error(const char *fmt,...){
	va_list ap;
	fprintf(stderr,"ERROR: ");
	va_start(ap,fmt);
	vfprintf(stderr,fmt,ap);
	va_end(ap);
	fprintf(stderr,"\n");
	exit(1);
}

void show(){
	struct dbt *dt=NULL;
	while((dt=dbtgetnxt(dt))){
		int n=0;
		struct dbf *df=NULL;
		while((df=dbfgetnxt(dt,df))) n++;
		printf("t %lu n %i\n",dbtgett(dt),n);
	}
}

int main(int argc,char **argv){
	char *dbf,*bdir,*cmd;
	if(argc<3) error("Usage: %s DBFILE BASEDIR (add|show)",argv[0]);
	dbf=argv[1];
	bdir=argv[2];
	cmd=argv[3];
	dbload(dbf);
	if(!strncmp(cmd,"add",3)){ add(bdir); dbsave(dbf); }
	if(!strncmp(cmd,"show",4)){ show(); }
	return 0;
}
