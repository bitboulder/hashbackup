#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ex.h"
#include "main.h"
#include "help.h"

struct ex {
	char pat[FNLEN];
	struct ex *nxt;
};

struct ex *exload(const char *fn){
	FILE *fd;
	char ex[2048],*p,*pat;
	struct ex *ret=NULL;
	if(!(fd=fopen(fn,"r"))) return NULL;
	if(!fgets(ex,sizeof(ex),fd)) return NULL;
	fclose(fd);
	ex[sizeof(ex)-1]='\0';
	fnrmnewline(ex);
	p=ex;
	while((pat=strsep(&p,"|"))){
		struct ex *ex=malloc(sizeof(struct ex));
		snprintf(ex->pat,FNLEN,"%s",pat);
		ex->nxt=ret;
		ret=ex;
	}
	return ret;
}

char exfn(struct ex *ex,const char *fn){
	char *p;
	for(;ex;ex=ex->nxt) if((p=strstr(fn,ex->pat))){
		size_t l=strlen(ex->pat);
		if(p>fn){
			if(ex->pat[0]=='/') continue;
			if(p[-1]!='/') continue;
		}
		if(p[l]=='/' || p[l]=='\0') return 1;
	}
	return 0;
}

