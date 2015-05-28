#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "dat.h"
#include "main.h"
#include "db.h"

#define BUFLEN	(1024*1024)

void shafn(const unsigned char *sha,char *fn){
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

void copyfile(const char *fni,const char *fno){
	FILE *fdi,*fdo;
	mkd(fno);
	if(!(fdi=fopen(fni,"rb"))){ error(1,"file open failed for '%s'",fni); return; }
	if(!(fdo=fopen(fno,"wb"))){ error(1,"file open failed for '%s'",fno); return; }
	while(!feof(fdi)){
		char buf[BUFLEN];
		size_t r=fread(buf,1,BUFLEN,fdi);
		fwrite(buf,1,r,fdo);
	}
	fclose(fdi);
	fclose(fdo);
}

void datadd(const unsigned char *sha,const char *fn){
	char fni[FNLEN],fno[FNLEN];
	struct stat st;
	shafn(sha,fno);
	if(!stat(fno,&st)) return;
	snprintf(fni,FNLEN,"%s/%s",dbbdir(),fn);
	copyfile(fni,fno);
}

void datget(const unsigned char *sha,const char *fno){
	char fni[FNLEN];
	shafn(sha,fni);
	copyfile(fni,fno);
}

