#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include "dirrec.h"
#include "main.h"

struct dri {
	struct dri *nxt;
	char fn[FNLEN];
	enum fmode mode;
};

struct dr {
	const char *bdir;
	struct ex *ex;
	struct dri *d;
};

void driadd(struct dri **dri,struct dri *d){
	d->nxt=*dri;
	*dri=d;
}

void drdir(struct dr *dr,const char *dir){
	char dn[FNLEN];
	DIR *dd;
	struct dirent *di;
	snprintf(dn,FNLEN,"%s/%s",dr->bdir,dir);
	if(!(dd=opendir(dn))){ error(0,"opendir failed for '%s'",dn); return; }
	/* TODO: sort by inode */
	while((di=readdir(dd))){
		struct dri *d;
		if(di->d_name[0]=='.' && (!di->d_name[1] || (di->d_name[1]=='.' && !di->d_name[2]))) continue;
		d=malloc(sizeof(struct dri));
		snprintf(d->fn,FNLEN,"%s/%s",dir,di->d_name);
		if(exfn(dr->ex,d->fn)){ free(d); continue; }
		switch(di->d_type){
		case DT_REG: d->mode=MS_FILE; break;
		case DT_DIR: d->mode=MS_DIR; break;
		case DT_LNK: d->mode=MS_LNK; break;
		default: d->mode=MS_NONE; break;
		}
		driadd(&dr->d,d);
	}
	closedir(dd);
}

int dirrec(const char *bdir,struct ex *ex,const char *dir,int (*fnc)(const char*,enum fmode,void *),void *arg){
	struct dr dr={
		.bdir=bdir,
		.ex=ex,
	};
	int ret=0;
	drdir(&dr,dir);
	while(dr.d){
		struct dri *d=dr.d;
		dr.d=d->nxt;
		if(d->mode==MS_DIR) drdir(&dr,d->fn);
		ret+=fnc(d->fn,d->mode,arg);
		free(d);
	}
	return ret;
}
