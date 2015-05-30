#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <utime.h>

#include "help.h"
#include "main.h"
#include "db.h"

char *fnrmnewline(char *fn){
	int i;
	for(i=0;i<FNLEN-1 && fn[i] && fn[i]!='\n';) i++;
	fn[i]='\0';
	return fn;
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
	if(bdir){ snprintf(ffn,FNLEN,"%s/%s",dbbdir(),fn); fn=ffn; }
	if(lstat(fn,&s)){
		error(0,"file stat failed for '%s'",fn);
		return 0;
	}
	memset(st,0,sizeof(struct st));
	if(S_ISREG(s.st_mode)) st->typ=FT_FILE;
	if(S_ISDIR(s.st_mode)) st->typ=FT_DIR;
	if(S_ISLNK(s.st_mode)) st->typ=FT_LNK;
	st->uid=s.st_uid;
	st->gid=s.st_gid;
	st->mode=s.st_mode;
	st->size=s.st_size;
	st->atime=s.st_atime;
	st->mtime=s.st_mtime;
	st->ctime=s.st_ctime;
	return 1;
}

void statset(struct st *st,const char *fn){
	struct utimbuf utim={.actime=st->atime, .modtime=st->mtime};
	lchown(fn,st->uid,st->gid);
	chmod(fn,st->mode); /* TODO: lchmod ?? */
	utime(fn,&utim); /* TODO: lutime ?? */
	/* TODO ctime */
	/* TODO check errors */
}

size_t filesize(const char *fn){
	struct stat st;
	if(!lstat(fn,&st)) return st.st_size;
	return 0;
}

void mkd(const char *fn){
	int i;
	char dn[FNLEN];
	for(i=0;i<FNLEN && fn[i];i++){
		struct stat st;
		dn[i]='\0';
		if(i && fn[i]=='/' && lstat(dn,&st)) mkdir(dn,0777);
		dn[i]=fn[i];
	}
}

void lnkget(const char *fn,char *lnk){
	char ffn[FNLEN];
	snprintf(ffn,FNLEN,"%s/%s",dbbdir(),fn);
	readlink(ffn,lnk,FNLEN);
}

void lnkset(const char *lnk,const char *fn){
	symlink(lnk,fn);
}

