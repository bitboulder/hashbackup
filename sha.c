#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <zlib.h>

#include "sha.h"
#include "main.h"
#include "dbt.h"

#define BUFLEN	8192
void shaget(const char *fn,unsigned char *sha){
	char ffn[FNLEN];
	FILE *fd;
	SHA_CTX c;
	snprintf(ffn,FNLEN,"%s/%s",dbbdir(),fn);
	if(!(fd=fopen(ffn,"rb"))){ error(0,"file open failed for '%s'",ffn); return; }
	SHA1_Init(&c);
	while(!feof(fd)){
		char buf[BUFLEN];
		size_t r=fread(buf,1,BUFLEN,fd);
		SHA1_Update(&c,buf,r);
	}
	SHA1_Final(sha,&c);
	fclose(fd);
}

void shagetdb(const char *fn,unsigned char *sha){
	gzFile fd;
	SHA_CTX c;
	if(!(fd=gzopen(fn,"rb"))){ error(0,"file open failed for '%s'",fn); return; }
	SHA1_Init(&c);
	while(!gzeof(fd)){
		char buf[BUFLEN];
		size_t r=gzread(fd,buf,BUFLEN);
		SHA1_Update(&c,buf,r);
	}
	SHA1_Final(sha,&c);
	gzclose(fd);
}

void shastr(const char *str,unsigned char *sha){
	size_t l=strlen(str);
	SHA((const unsigned char*)str,l,sha);
}

void shabuf(const unsigned char *buf,size_t l,unsigned char *sha){
	SHA(buf,l,sha);
}

void sha2fn(const unsigned char *sha,char *fn){
	int f,s;
	f=snprintf(fn,FNLEN,"%s",DH);
	fn[f++]='/';
	for(s=0;s<SHALEN*2;s++){
		unsigned char c= s%2 ? *sha&0xf : *sha>>4;
		fn[f++] = c>=10 ? c-10+'a' : c+'0';
		if(s%2) sha++;
		if(s==1) fn[f++]='/';
		if(s==3) fn[f++]='/';
	}
	fn[f++]='\0';
}

void fn2sha(const char *fn,unsigned char *sha){
	int f=0,s;
	if(!strncmp(fn,DH "/",4)) f=4;
	for(s=0;fn[f] && s<SHALEN*2;f++) if((fn[f]>='0' && fn[f]<='9')||(fn[f]>='a' && fn[f]<='f')){
		unsigned char c = fn[f]>='a' ? fn[f]-'a'+10 : fn[f]-'0';
		if(s%2){ *sha+=c; sha++; }else *sha=c<<4;
		s++;
	}
}

