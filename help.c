#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "help.h"
#include "main.h"
#include "db.h"

int dirrec(const char *bdir,struct ex *ex,const char *dir,int (*fnc)(const char*,enum fmode,void *),void *arg){
	char dn[FNLEN];
	DIR *dd;
	struct dirent *di;
	int ret=0;
	snprintf(dn,FNLEN,"%s/%s",bdir,dir);
	if(!(dd=opendir(dn))){ error(0,"opendir failed for '%s'",dn); return 0; }
	/* TODO: sort by inode */
	while((di=readdir(dd))){
		char fn[FNLEN];
		enum fmode mode;
		if(di->d_name[0]=='.' && (!di->d_name[1] || (di->d_name[1]=='.' && !di->d_name[2]))) continue;
		snprintf(fn,FNLEN,"%s/%s",dir,di->d_name);
		if(exfn(ex,fn)) continue;
		switch(di->d_type){
		case DT_REG: mode=MS_FILE; break;
		case DT_DIR: mode=MS_DIR; break;
		case DT_LNK: mode=MS_LNK; break;
		default: mode=MS_NONE; break;
		}
		ret+=fnc(fn,mode,arg);
		if(mode==MS_DIR) ret+=dirrec(bdir,ex,fn,fnc,arg);
	}
	closedir(dd);
	return ret;
}

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

