#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zlib.h>

#include "dat.h"
#include "main.h"
#include "db.h"
#include "help.h"

#define BUFLEN	(1024*1024)

void sha2fn(const unsigned char *sha,char *fn){
	snprintf(fn,FNLEN,DH "/%02x/%02x/%016lx%016lx%02x",
		sha[0],sha[1],
		*(uint64_t*)(sha+2),
		*(uint64_t*)(sha+10),
		*(uint16_t*)(sha+18));
}

void mkd(const char *fn){
	int i;
	char dn[FNLEN];
	for(i=0;i<FNLEN && fn[i];i++){
		struct stat st;
		dn[i]='\0';
		if(i && fn[i]=='/' && stat(dn,&st)) mkdir(dn,0777);
		dn[i]=fn[i];
	}
}

void datadd(const unsigned char *sha,const char *fn){
	char fni[FNLEN],fno[FNLEN];
	struct stat st;
	FILE *fdi;
	gzFile fdo;
	sha2fn(sha,fno);
	if(!stat(fno,&st)) return;
	snprintf(fni,FNLEN,"%s/%s",dbbdir(),fn);
	mkd(fno);
	if(!(fdi=fopen(fni,"rb"))){ error(1,"file open failed for '%s'",fni); return; }
	if(!(fdo=gzopen(fno,"wb"))){ error(1,"file open failed for '%s'",fno); return; }
	while(!feof(fdi)){
		char buf[BUFLEN];
		size_t r=fread(buf,1,BUFLEN,fdi);
		gzwrite(fdo,buf,r);
	}
	fclose(fdi);
	gzclose(fdo);
}

void datget(const unsigned char *sha,const char *fno){
	char fni[FNLEN];
	gzFile fdi;
	FILE *fdo;
	sha2fn(sha,fni);
	mkd(fno);
	if(!(fdi=gzopen(fni,"rb"))){ error(1,"file open failed for '%s'",fni); return; }
	if(!(fdo=fopen(fno,"wb"))){ error(1,"file open failed for '%s'",fno); return; }
	while(!gzeof(fdi)){
		char buf[BUFLEN];
		size_t r=gzread(fdi,buf,BUFLEN);
		fwrite(buf,1,r,fdo);
	}
	gzclose(fdi);
	fclose(fdo);
}

void datdel(const unsigned char *sha){
	char fn[FNLEN];
	sha2fn(sha,fn);
	unlink(fn);
}

size_t datsi(const unsigned char *sha){
	char fn[FNLEN];
	struct st st;
	sha2fn(sha,fn);
	if(statget(0,fn,&st)) return st.size;
	error(0,"dat file missing: '%s'",fn);
	return 0;
}
