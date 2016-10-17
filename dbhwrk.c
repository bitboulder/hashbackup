#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <zlib.h>

#include "dbhwrk.h"
#include "main.h"
#include "dbt.h"
#include "help.h"
#include "sha.h"
#include "mc.h"

#define BUFLEN	8192

size_t dbhsave(const unsigned char *sha,const char *fn){
	char fni[FNLEN],fno[FNLEN];
	sha2fn(sha,fno);
	snprintf(fni,FNLEN,"%s/%s",dbbdir(),fn);
	mkd(fno);
	if(mc_nozip(fn)){
		FILE *fdi;
		FILE *fdo;
		if(!(fdi=fopen(fni,"rb"))){ error(1,"file open failed for '%s'",fni); return 0; }
		if(!(fdo=fopen(fno,"wb"))){ error(1,"file open failed for '%s'",fno); return 0; }
		while(!feof(fdi)){
			char buf[BUFLEN];
			size_t r=fread(buf,1,BUFLEN,fdi);
			fwrite(buf,1,r,fdo);
		}
		fclose(fdi);
		fclose(fdo);
	}else{
		FILE *fdi;
		gzFile fdo;
		if(!(fdi=fopen(fni,"rb"))){ error(1,"file open failed for '%s'",fni); return 0; }
		if(!(fdo=gzopen(fno,"wb"))){ error(1,"file open failed for '%s'",fno); return 0; }
		while(!feof(fdi)){
			char buf[BUFLEN];
			size_t r=fread(buf,1,BUFLEN,fdi);
			gzwrite(fdo,buf,r);
		}
		fclose(fdi);
		gzclose(fdo);
	}
	return filesize(fno);
}

size_t dbhsavebuf(const unsigned char *sha,const unsigned char *buf,size_t l){
	char fno[FNLEN];
	gzFile fdo;
	sha2fn(sha,fno);
	mkd(fno);
	if(!(fdo=gzopen(fno,"wb"))){ error(1,"file open failed for '%s'",fno); return 0; }
	gzwrite(fdo,buf,l);
	gzclose(fdo);
	return filesize(fno);
}

void dbhrestore(const unsigned char *sha,const char *fno){
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

void dbhrestorebuf(const unsigned char *sha,unsigned char *buf,size_t l){
	char fni[FNLEN];
	gzFile fdi;
	sha2fn(sha,fni);
	if(!(fdi=gzopen(fni,"rb"))){ error(1,"file open failed for '%s'",fni); return; }
	gzread(fdi,buf,l);
	gzclose(fdi);
}

void dbhdel(const unsigned char *sha){
	char fn[FNLEN];
	sha2fn(sha,fn);
	unlink(fn);
}

