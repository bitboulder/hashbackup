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
	char fni[FNFLEN];
	struct str fno=STRDEF;
	size_t ret;
	str_setlen(&fno,FNFLEN);
	sha2fn(sha,&fno);
	snprintf(fni,sizeof(fni),"%s/%s",dbbdir(),fn);
	mkd(fno.s);
	if(mc_nozip(fn)){
		FILE *fdi;
		FILE *fdo;
		if(!(fdi=fopen(fni,"rb"))){ error(1,"file open failed for '%s'",fni); return 0; }
		if(!(fdo=fopen(fno.s,"wb"))){ error(1,"file open failed for '%s'",fno.s); return 0; }
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
		if(!(fdo=gzopen(fno.s,"wb"))){ error(1,"file open failed for '%s'",fno.s); return 0; }
		while(!feof(fdi)){
			char buf[BUFLEN];
			size_t r=fread(buf,1,BUFLEN,fdi);
			gzwrite(fdo,buf,r);
		}
		fclose(fdi);
		gzclose(fdo);
	}
	ret=filesize(fno.s);
	str_setlen(&fno,0);
	return ret;
}

size_t dbhsavebuf(const unsigned char *sha,const unsigned char *buf,size_t l){
	struct str fno;
	size_t ret;
	str_setlen(&fno,FNFLEN);
	gzFile fdo;
	sha2fn(sha,&fno);
	mkd(fno.s);
	if(!(fdo=gzopen(fno.s,"wb"))){ error(1,"file open failed for '%s'",fno.s); return 0; }
	gzwrite(fdo,buf,l);
	gzclose(fdo);
	ret=filesize(fno.s);
	str_setlen(&fno,0);
	return ret;
}

void dbhrestore(const unsigned char *sha,const char *fno){
	struct str fni;
	gzFile fdi;
	FILE *fdo;
	str_setlen(&fni,FNFLEN);
	sha2fn(sha,&fni);
	mkd(fno);
	if(!(fdi=gzopen(fni.s,"rb"))){ error(1,"file open failed for '%s'",fni.s); return; }
	if(!(fdo=fopen(fno,"wb"))){ error(1,"file open failed for '%s'",fno); return; }
	while(!gzeof(fdi)){
		char buf[BUFLEN];
		size_t r=gzread(fdi,buf,BUFLEN);
		fwrite(buf,1,r,fdo);
	}
	gzclose(fdi);
	fclose(fdo);
	str_setlen(&fni,0);
}

void dbhrestorebuf(const unsigned char *sha,unsigned char *buf,size_t l){
	struct str fni;
	gzFile fdi;
	str_setlen(&fni,FNFLEN);
	sha2fn(sha,&fni);
	if(!(fdi=gzopen(fni.s,"rb"))){ error(1,"file open failed for '%s'",fni.s); return; }
	gzread(fdi,buf,l);
	gzclose(fdi);
	str_setlen(&fni,0);
}

void dbhdel(const unsigned char *sha){
	struct str fn;
	str_setlen(&fn,FNFLEN);
	sha2fn(sha,&fn);
	unlink(fn.s);
	str_setlen(&fn,0);
}

