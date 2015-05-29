#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <zlib.h>

#include "dat.h"
#include "main.h"
#include "db.h"
#include "help.h"

#define BUFLEN	8192

void sha2fn(const unsigned char *sha,char *fn){
	snprintf(fn,FNLEN,DH "/%02x/%02x/%016lx%016lx%02x",
		sha[0],sha[1],
		*(uint64_t*)(sha+2),
		*(uint64_t*)(sha+10),
		*(uint16_t*)(sha+18));
}

size_t datadd(const unsigned char *sha,const char *fn){
	char fni[FNLEN],fno[FNLEN];
	FILE *fdi;
	gzFile fdo;
	sha2fn(sha,fno);
	snprintf(fni,FNLEN,"%s/%s",dbbdir(),fn);
	mkd(fno);
	if(!(fdi=fopen(fni,"rb"))){ error(1,"file open failed for '%s'",fni); return 0; }
	if(!(fdo=gzopen(fno,"wb"))){ error(1,"file open failed for '%s'",fno); return 0; }
	while(!feof(fdi)){
		char buf[BUFLEN];
		size_t r=fread(buf,1,BUFLEN,fdi);
		gzwrite(fdo,buf,r);
	}
	fclose(fdi);
	gzclose(fdo);
	return filesize(fno);
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
	size_t si;
	sha2fn(sha,fn);
	si=filesize(fn);
	if(!si) error(0,"dat file missing: '%s'",fn);
	return 0;
}
