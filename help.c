#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
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

const char *timefmt(time_t t){
	static char buf[128];
	strftime(buf,sizeof(buf),"%y-%m-%d %H:%M",localtime(&t));
	return buf;
}

const char *sizefmt(size_t si){
	static char buf[32];
	double s=si;
	char *e=" kMGT",fmt[32];
	int k=0;
	while(s>=1000 && e[0]){ s/=1024; e++; }
	if(s<100) k++;
	if(s<10) k++;
	snprintf(fmt,sizeof(fmt),"%%.%if%%c",k);
	snprintf(buf,sizeof(buf),fmt,s,e[0]);
	return buf;
}

char mstat(const char *fn,struct mstat *st){
	char ffn[FNLEN];
	struct stat s;
	snprintf(ffn,FNLEN,"%s/%s",dbbdir(),fn);
	if(stat(ffn,&s)){
		error(0,"file stat failed for '%s'",ffn);
		return 0;
	}
	memset(st,0,sizeof(struct mstat));
	if(S_ISREG(s.st_mode)) st->mode|=MS_FILE;
	if(S_ISDIR(s.st_mode)) st->mode|=MS_DIR;
	if(S_ISLNK(s.st_mode)) st->mode|=MS_LNK;
	st->uid=s.st_uid;
	st->gid=s.st_gid;
	st->size=s.st_size;
	st->mtime=s.st_mtime;
	st->ctime=s.st_ctime;
	return 1;
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
	fclose(fd);
	return 0;
}

