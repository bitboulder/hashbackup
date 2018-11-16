#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <ext2fs.h>
#include <zlib.h>

#include "ext2.h"
#include "main.h"
#include "dbt.h"
#include "sha.h"
#include "dbhwrk.h"
#include "help.h"

struct dbe {
	size_t bsize,bnum;
	struct dbh **h;
};

char ext2read(struct dbf *df,struct dbt *dt){
	char ffn[FNFLEN];
	char ret=0;
	ext2_filsys fs=NULL;
	size_t blk;
	struct dbe *de;
	unsigned char *buf;
	snprintf(ffn,sizeof(ffn),"%s/%s",dbbdir(),dbfgetfn(df));
	if(ext2fs_open(ffn,0,0,0,unix_io_manager,&fs)) return ret;
	if(ext2fs_read_inode_bitmap(fs)) goto end;
	if(ext2fs_read_block_bitmap(fs)) goto end;
	ret=1;
	de=malloc(sizeof(struct dbe));
	buf=malloc(de->bsize=fs->blocksize);
	de->h=calloc(de->bnum=fs->super->s_blocks_count,sizeof(struct dbh*));
	for(blk=0;blk<de->bnum;blk++)
		if(blk<fs->super->s_first_data_block || ext2fs_test_block_bitmap(fs->block_map,blk)){
			unsigned char sha[SHALEN];
			struct dbh *dh;
			io_channel_read_blk(fs->io,blk,1,buf);
			shabuf(buf,de->bsize,sha);
			if(!(dh=dbhget(sha))){
				dh=dbhnew(sha,0);
				dbhsavebuf(sha,buf,de->bsize);
			}
			dbhadd(dh,dt,df);
			de->h[blk]=dh;
		}
	dbfsetext2(df,de);
	free(buf);
end:
	ext2fs_close(fs);
	return ret;
}

void ext2restore(struct dbf *df,const char *fn){
	FILE *fd;
	struct dbe *de=dbfgetext2(df);
	size_t blk;
	mkd(fn);
	unsigned char *zero,*buf;
	if(!(fd=fopen(fn,"wb"))) error(1,"file open failed for '%s'",fn); 
	zero=calloc(1,de->bsize);
	buf=malloc(de->bsize);
	for(blk=0;blk<de->bnum;blk++)
		if(de->h[blk]){
			dbhrestorebuf(dbhgetsha(de->h[blk]),buf,de->bsize);
			fwrite(buf,1,de->bsize,fd);
		}else fwrite(zero,1,de->bsize,fd);
	if(de->bnum*de->bsize<dbfgetst(df)->size){
		size_t r=dbfgetst(df)->size-de->bnum*de->bsize;
		while(r) r-=fwrite(zero,1,MIN(r,de->bsize),fd);
	}
	fclose(fd);
	free(zero);
	free(buf);
}

void ext2copy(struct dbf *df,struct dbt *dt,struct dbe *de){
	size_t blk;
	dbfsetext2(df,de);
	for(blk=0;blk<de->bnum;blk++) if(de->h[blk]) dbhadd(de->h[blk],dt,df);
}

struct dbe *ext2load(void *fd,struct dbt *dt,struct dbf *df){
	struct dbe *de=malloc(sizeof(struct dbe));
	size_t blk;
	gzFile *gd=(gzFile*)fd;
	gzread(*gd,&de->bsize,sizeof(size_t));
	gzread(*gd,&de->bnum,sizeof(size_t));
	de->h=calloc(de->bnum,sizeof(struct dbh*));
	for(blk=0;blk<de->bnum;blk++){
		unsigned char sha[SHALEN];
		int i=0;
		gzread(*gd,sha,SHALEN);
		while(i<SHALEN && !sha[i]) i++;
		if(i<SHALEN){
			if(!(de->h[blk]=dbhget(sha))){
				struct str fn=STRDEF;
				str_setlen(&fn,FNFLEN);
				sha2fn(sha,&fn);
				error(0,"dbh file missing: '%s'",fn.s);
				str_setlen(&fn,0);
			}else dbhadd(de->h[blk],dt,df);
		}
	}
	return de;
}

void ext2save(struct dbe *de,void *fd){
	size_t blk;
	unsigned char zero[SHALEN];
	memset(zero,0,SHALEN);
	gzFile *gd=(gzFile*)fd;
	gzwrite(*gd,&de->bsize,sizeof(size_t));
	gzwrite(*gd,&de->bnum,sizeof(size_t));
	for(blk=0;blk<de->bnum;blk++)
		gzwrite(*gd,de->h[blk] ? dbhgetsha(de->h[blk]) : zero,SHALEN);
}

void ext2list(struct dbe *de){
	size_t blk;
	for(blk=0;blk<de->bnum;blk++) if(de->h[blk]){
		unsigned char *sha=dbhgetsha(de->h[blk]);
		int i;
		printf("    ");
		for(i=0;i<4;i++) printf("%02x",sha[i]);
		printf(" %5s",sizefmt(de->bsize));
		printf(" %5s",sizefmt(dbhgetsi(de->h[blk])));
		printf(" 0x%06lx\n",blk);
	}
		
}

