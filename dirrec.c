#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include "dirrec.h"
#include "main.h"

struct dri {
	size_t ino;
	char fn[FNLEN];
	enum ftyp typ;
};

struct dris {
	size_t used;
	size_t size;
	struct dri **d;
};

struct dr {
	const char *bdir;
	struct ex *ex;
	struct dris dcur,dnxt;
};

void drisinit(struct dris *ds){
	ds->used=0;
	ds->size=1023;
	ds->d=malloc((ds->size+1)*sizeof(struct dri*));
}

void drisresize(struct dris *ds){
	ds->size+=ds->size+1;
	ds->d=realloc(ds->d,(ds->size+1)*sizeof(struct dri*));
}

void drisheapify(struct dris *ds,struct dri *d,size_t pn){
	register size_t p=pn;
	register size_t ino=d->ino;
	while(p>1){
		register size_t p2=p>>1;
		if(ds->d[p2]->ino<=ino) break;
		ds->d[p]=ds->d[p2];
		p=p2;
	}
	while(1){
		register size_t i2,i3;
		register size_t p2=p<<1,p3=p2+1;
		if(p2>ds->used) break;
		i2=ds->d[p2]->ino;
		i3=p3>ds->used ? SIZE_MAX : ds->d[p3]->ino;
		if(i3<ino){ if(i2>=i3) p2=p3; }else if(i2>=ino) break;
		ds->d[p]=ds->d[p2];
		p=p2;
	}
	ds->d[p]=d;
}

void drisput(struct dris *ds,struct dri *d){
	if(ds->used==ds->size) drisresize(ds);
	drisheapify(ds,d,++ds->used);
}

struct dri *drispop(struct dris *ds){
	struct dri *d=ds->d[1];
	if(!ds->used) return NULL;
	drisheapify(ds,ds->d[ds->used--],1);
	return d;
}

void drdir(struct dr *dr,const char *dir,size_t ino){
	char dn[FNLEN];
	DIR *dd;
	struct dirent *di;
	snprintf(dn,FNLEN,"%s/%s",dr->bdir,dir);
	if(!(dd=opendir(dn))){ error(0,"opendir failed for '%s'",dn); return; }
	while((di=readdir(dd))){
		struct dri *d;
		if(di->d_name[0]=='.' && (!di->d_name[1] || (di->d_name[1]=='.' && !di->d_name[2]))) continue;
		d=malloc(sizeof(struct dri));
		snprintf(d->fn,FNLEN,"%s/%s",dir,di->d_name);
		if(exfn(dr->ex,d->fn)){ free(d); continue; }
		switch(di->d_type){
		case DT_REG: d->typ=FT_FILE; break;
		case DT_DIR: d->typ=FT_DIR; break;
		case DT_LNK: d->typ=FT_LNK; break;
		default: d->typ=FT_NONE; break;
		}
		d->ino=di->d_ino;
		drisput(d->ino<ino ? &dr->dnxt : &dr->dcur,d);
	}
	closedir(dd);
}

int dirrec(const char *bdir,struct ex *ex,const char *dir,int (*fnc)(const char*,enum ftyp,void *),void *arg){
	struct dr dr={
		.bdir=bdir,
		.ex=ex,
	};
	int ret=0;
	drisinit(&dr.dcur);
	drisinit(&dr.dnxt);
	drdir(&dr,dir,0);
	do{
		struct dri *d;
		struct dris ds;
		while((d=drispop(&dr.dcur))){
			if(d->typ==FT_DIR) drdir(&dr,d->fn,d->ino);
			ret+=fnc(d->fn,d->typ,arg);
			free(d);
		}
		ds=dr.dcur;
		dr.dcur=dr.dnxt;
		dr.dnxt=ds;
	}while(dr.dcur.used);
	return ret;
}
