#define _GNU_SOURCE
#ifdef __CYGWIN__
	#undef __STRICT_ANSI__
#endif
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>

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
	time_t t=atoi(stime);
	struct dbt *dt=dbtget(t);
	if(!dt){
		struct tm tm;
		struct dbt *dti=dt=dbtgetnxt(NULL);
		memset(&tm,0,sizeof(struct tm));
		if(!dt) error(1,"not time found: '%s' -> %li\n",stime,t);
		if(
		   !strptime(stime,"%y-%m-%d %H:%M",&tm) &&
		   !strptime(stime,"%y-%m-%d",&tm) &&
		   !strptime(stime,"%y-%m",&tm) &&
		   !strptime(stime,"%y",&tm)
		  ) error(1,"time not parseable: '%s' -> %li\n",stime,t);
		t=mktime(&tm);
		while((dti=dbtgetnxt(dti))) if(labs((long)dbtgett(dti)-(long)t)<labs((long)dbtgett(dt)-(long)t)) dt=dti;
	}
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
	struct timeval utim[2]={
		{.tv_sec=st->atime,.tv_usec=0},
		{.tv_sec=st->mtime,.tv_usec=0},
	};
	if(lchown(fn,st->uid,st->gid)) error(0,"lchown failed for '%s': %s",fn,strerror(errno));
	if(st->typ!=FT_LNK && chmod(fn,st->mode)) error(0,"chmod failed for '%s'",fn);
	if(lutimes(fn,utim)) error(0,"lutimes failed for '%s'",fn);
}

enum statcmp statcmp(struct st *a,struct st *b){
	enum statcmp sd=SD_EQL;
	if(a->typ!=b->typ) sd|=SD_TYP;
	if(a->uid!=b->uid) sd|=SD_UID;
	if(a->gid!=b->gid) sd|=SD_GID;
	if(a->mode!=b->mode) sd|=SD_MODE;
	if(a->size!=b->size) sd|=SD_SIZE;
	if(a->mtime!=b->mtime) sd|=SD_MTIME;
	if(a->ctime!=b->ctime) sd|=SD_CTIME;
	return sd;
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

