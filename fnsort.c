#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fnsort.h"
#include "main.h"

struct fns {
	size_t size;
	size_t used;
	char sort;
	struct fn {
		const char *fn;
		int arg;
	} *fn;
};

struct fns *fnsinit(){
	return calloc(1,sizeof(struct fns));
}

void fnsadd(struct fns *fns,const char *fn,int arg){
	if(fns->size==fns->used) fns->fn=realloc(fns->fn,(fns->size+=1024)*sizeof(struct fn));
	fns->fn[fns->used++]=(struct fn){.fn=fn,.arg=arg};
	fns->sort=0;
}

int fnscmp(const void *a,const void *b){ return -strncmp(((struct fn*)a)->fn,((struct fn*)b)->fn,FNLEN); }

const char *fnsnxt(struct fns *fns,int *arg){
	struct fn fn;
	if(!fns->used) return NULL;
	if(!fns->sort) qsort(fns->fn,fns->used,sizeof(struct fn),fnscmp);
	fn=fns->fn[--fns->used];
	if(!fns->used){
		free(fns->fn);
		fns->size=0;
	}
	if(arg) *arg=fn.arg;
	return fn.fn;
}

