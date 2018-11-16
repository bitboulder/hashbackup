#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ex.h"
#include "main.h"
#include "help.h"

struct ex {
	char pat[FNFLEN];
	struct ex *nxt;
};

struct ex *exload(const char *fn){
	FILE *fd;
	struct str ex=STRDEF;
	char *p,*pat;
	struct ex *ret=NULL;
	str_setlen(&ex,2048);
	if(!(fd=fopen(fn,"r"))) goto end;
	if(!fgets(ex.s,ex.l,fd)) goto end;
	fclose(fd);
	ex.s[ex.l-1]='\0';
	fnrmnewline(&ex);
	p=ex.s;
	while((pat=strsep(&p,"|"))){
		struct ex *ex=malloc(sizeof(struct ex));
		snprintf(ex->pat,sizeof(ex->pat),"%s",pat);
		ex->nxt=ret;
		ret=ex;
	}
end:
	str_setlen(&ex,0);
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

