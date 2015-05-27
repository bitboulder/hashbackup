#include <stdio.h>
#include <stdlib.h>

#include "cmd.h"
#include "main.h"
#include "db.h"

void init(const char *basedir){
	FILE *fd;
	system("mkdir " DD " " DH);
	if(!(fd=fopen("basedir","w"))) error("basedir open failed");
	fprintf(fd,"%s\n",basedir);
	fclose(fd);
}

void diff(const char *stime){
}

void cifile(void *dtv){
}

void commit(){
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

void flist(const char *stime){
}

void del(const char *stime){
}

