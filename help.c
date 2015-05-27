#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <openssl/sha.h>

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

char mstat(const char *fn,struct stat *st){
	char ffn[FNLEN];
	snprintf(ffn,FNLEN,"%s/%s",dbbdir(),fn);
	if(!stat(ffn,st)) return 1;
	error(0,"file stat failed for '%s'",ffn);
	return 0;
}

#define BUFLEN	(1024*1024)
char msha(const char *fn,unsigned char *sha){
	char ffn[FNLEN];
	FILE *fd;
	SHA_CTX c;
	snprintf(ffn,FNLEN,"%s/%s",dbbdir(),fn);
	if(!(fd=fopen(ffn,"rb"))){ error(0,"file open failed for '%s'",ffn); return 0; }
	SHA1_Init(&c);
	while(!feof(fd)){
		char buf[BUFLEN];
		size_t r=fread(buf,1,BUFLEN,fd);
		SHA1_Update(&c,buf,r);
	}
	SHA1_Final(sha,&c);
	return 0;
}
