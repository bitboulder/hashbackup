#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "help.h"
#include "main.h"
#include "db.h"

void dirrec(const char *bdir,const char *dir,void (*fnc)(const char*,void *),void *arg){
	char dn[FNLEN];
	DIR *dd;
	struct dirent *di;
	snprintf(dn,FNLEN,"%s/%s",bdir,dir);
	if(!(dd=opendir(dn))){ error(0,"opendir failed for '%s'",dn); return; }
	while((di=readdir(dd))){
		char fn[FNLEN];
		if(di->d_name[0]=='.' && (!di->d_name[1] || (di->d_name[1]=='.' && !di->d_name[2]))) continue;
		snprintf(fn,FNLEN,"%s/%s",dir,di->d_name);
		fnc(fn,arg);
		if((di->d_type&DT_DIR) && !(di->d_type&DT_LNK)) dirrec(bdir,fn,fnc,arg);
	}
	closedir(dd);
}

struct dbt *timeparse(const char *stime){
	time_t t=atoi(stime); /* TODO: real parse */
	struct dbt *dt=dbtget(t);
	if(!dt) error(1,"time not found: '%s' -> %li\n",stime,t);
	return dt;
}

struct stat *mstat(const char *fn){
	char ffn[FNLEN];
	static struct stat st;
	snprintf(ffn,FNLEN,"%s/%s",dbbdir(),fn);
	if(!stat(ffn,&st)) return &st;
	error(0,"file stat failed for '%s'",ffn);
	return NULL;
}
