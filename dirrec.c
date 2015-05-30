#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include "dirrec.h"
#include "main.h"

int dirrec(const char *bdir,struct ex *ex,const char *dir,int (*fnc)(const char*,enum fmode,void *),void *arg){
	char dn[FNLEN];
	DIR *dd;
	struct dirent *di;
	int ret=0;
	snprintf(dn,FNLEN,"%s/%s",bdir,dir);
	if(!(dd=opendir(dn))){ error(0,"opendir failed for '%s'",dn); return 0; }
	/* TODO: sort by inode */
	while((di=readdir(dd))){
		char fn[FNLEN];
		enum fmode mode;
		if(di->d_name[0]=='.' && (!di->d_name[1] || (di->d_name[1]=='.' && !di->d_name[2]))) continue;
		snprintf(fn,FNLEN,"%s/%s",dir,di->d_name);
		if(exfn(ex,fn)) continue;
		switch(di->d_type){
		case DT_REG: mode=MS_FILE; break;
		case DT_DIR: mode=MS_DIR; break;
		case DT_LNK: mode=MS_LNK; break;
		default: mode=MS_NONE; break;
		}
		ret+=fnc(fn,mode,arg);
		if(mode==MS_DIR) ret+=dirrec(bdir,ex,fn,fnc,arg);
	}
	closedir(dd);
	return ret;
}

