#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include "help.h"
#include "main.h"

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
