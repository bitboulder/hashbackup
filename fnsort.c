#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fnsort.h"
#include "main.h"

struct fns {
	size_t size;
	size_t used;
	char sort;
	const char *free;
	struct fn {
		const char *fn;
		char cp;
		void *arg;
	} *fn;
};

struct fns *fnsinit(){
	return calloc(1,sizeof(struct fns));
}

void fnsadd(struct fns *fns,const char *fn,char cp,void *arg){
	if(fns->size==fns->used) fns->fn=realloc(fns->fn,(fns->size+=1024)*sizeof(struct fn));
	if(cp){
		char *fnc=malloc(FNLEN);
		snprintf(fnc,FNLEN,"%s",fn);
		fn=fnc;
	}
	fns->fn[fns->used++]=(struct fn){.fn=fn,.arg=arg,.cp=cp};
	fns->sort=0;
}

int fnscmp(const void *a,const void *b){ return -strncmp(((struct fn*)a)->fn,((struct fn*)b)->fn,FNLEN); }

const char *fnsnxt(struct fns *fns,void **arg){
	struct fn fn;
	if(fns->free){ free((char*)fns->free); fns->free=NULL; }
	if(!fns->used) return NULL;
	if(!fns->sort) qsort(fns->fn,fns->used,sizeof(struct fn),fnscmp);
	fn=fns->fn[--fns->used];
	if(!fns->used){
		free(fns->fn);
		fns->size=0;
	}
	if(arg) *arg=fn.arg;
	if(fn.cp) fns->free=fn.fn;
	return fn.fn;
}

