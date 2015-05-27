#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include "add.h"
#include "main.h"
#include "db.h"

void adddir(struct dbt *dt,const char *bdir,const char *dir){
	DIR *dd;
	char dn[FNLEN];
	struct dirent *di;
	snprintf(dn,FNLEN,"%s/%s",bdir,dir);
	if(!(dd=opendir(dn))) return;
	while((di=readdir(dd))){
		char fn[FNLEN];
		struct dbf *df;
		if(di->d_name[0]=='.' && (!di->d_name[1] || (di->d_name[1]=='.' && !di->d_name[2]))) continue;
		snprintf(fn,FNLEN,"%s/%s",dir,di->d_name);
		df=dbfnew(dt,fn);
		if(di->d_type&DT_DIR) adddir(dt,bdir,fn);
	}
	closedir(dd);
}

void add(const char *bdir){
	adddir(dt,bdir,"");
}

