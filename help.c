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

int dirrec(const char *bdir,char ex,const char *dir,int (*fnc)(const char*,void *),void *arg){
	char dn[FNLEN];
	DIR *dd;
	struct dirent *di;
	int ret=0;
	snprintf(dn,FNLEN,"%s/%s",bdir,dir);
	if(!(dd=opendir(dn))){ error(0,"opendir failed for '%s'",dn); return 0; }
	while((di=readdir(dd))){
		char fn[FNLEN];
		if(di->d_name[0]=='.' && (!di->d_name[1] || (di->d_name[1]=='.' && !di->d_name[2]))) continue;
		snprintf(fn,FNLEN,"%s/%s",dir,di->d_name);
		if(ex && dbex(fn)) continue;
		ret+=fnc(fn,arg);
		if((di->d_type&DT_DIR) && !(di->d_type&DT_LNK)) ret+=dirrec(bdir,ex,fn,fnc,arg);
	}
	closedir(dd);
	return ret;
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

char statget(char bdir,const char *fn,struct st *st){
	char ffn[FNLEN];
	struct stat s;
	if(bdir){
		snprintf(ffn,FNLEN,"%s/%s",dbbdir(),fn);
		fn=ffn;
	}
	if(lstat(fn,&s)){
		error(0,"file stat failed for '%s'",fn);
		return 0;
	}
	memset(st,0,sizeof(struct st));
	if(S_ISREG(s.st_mode)) st->mode=MS_FILE;
	if(S_ISDIR(s.st_mode)) st->mode=MS_DIR;
	if(S_ISLNK(s.st_mode)) st->mode=MS_LNK;
	st->uid=s.st_uid;
	st->gid=s.st_gid;
	st->size=s.st_size;
	st->mtime=s.st_mtime;
	st->ctime=s.st_ctime;
	return 1;
}

void statset(struct st *st,const char *fn){
	/* TODO */
}

void lnkget(const char *fn,char *lnk){
	char ffn[FNLEN];
	snprintf(ffn,FNLEN,"%s/%s",dbbdir(),fn);
	readlink(ffn,lnk,FNLEN);
}

void lnkset(const char *lnk,const char *fn){
	symlink(lnk,fn);
}

#define BUFLEN	(1024*1024)
char shaget(const char *fn,unsigned char *sha){
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

void shafn(const char *fn,unsigned char *sha){
	size_t l=strlen(fn);
	SHA((const unsigned char*)fn,l,sha);
}
