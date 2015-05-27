#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "dat.h"
#include "main.h"
#include "db.h"

#define BUFLEN	(1024*1024)
void datadd(const unsigned char *sha,const char *fn){
	char ffn[FNLEN],ffo[FNLEN];
	FILE *fdi,*fdo;
	snprintf(ffo,FNLEN,DH "/%02x/%02x/%016lx%016lx%02x",
		sha[0],sha[1],
		*(uint64_t*)(sha+2),
		*(uint64_t*)(sha+10),
		*(uint16_t*)(sha+18));
	if((fdo=fopen(ffo,"rb"))){ fclose(fdo); return; }
	snprintf(ffn,FNLEN,"%s/%s",dbbdir(),fn);
	if(!(fdi=fopen(ffn,"rb"))){ error(1,"file open failed for '%s'",ffn); return; }
	snprintf(ffn,FNLEN,"mkdir -p " DH "/%02x",sha[0]); system(ffn);
	snprintf(ffn,FNLEN,"mkdir -p " DH "/%02x/%02x",sha[0],sha[1]); system(ffn);
	if(!(fdo=fopen(ffo,"wb"))){ error(1,"file open failed for '%s'",ffn); return; }
	while(!feof(fdi)){
		char buf[BUFLEN];
		size_t r=fread(buf,1,BUFLEN,fdi);
		fwrite(buf,1,r,fdo);
	}
	fclose(fdi);
	fclose(fdo);
}
